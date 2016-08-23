#include <QSettings>
#include "ViewRecord.h"
#include "ui_ViewRecord.h"
#include "qt_util.h"


ViewRecord::ViewRecord(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ViewRecord)
{
  ui->setupUi(this);

  ui->plotRecord->set_antialiased(false);
  ui->plotRecord->set_scale_type("Linear");
  ui->plotRecord->set_show_legend(true);

  ui->plotProjection->set_scale_type("Linear");
  ui->plotProjection->set_plot_style("Step center");
  ui->plotProjection->set_visible_options(ShowOptions::thickness | ShowOptions::grid | ShowOptions::save);


  ui->tableValues->verticalHeader()->hide();
  ui->tableValues->setColumnCount(2);
  ui->tableValues->setHorizontalHeaderLabels({"name", "value"});
  ui->tableValues->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableValues->setSelectionMode(QAbstractItemView::NoSelection);
//  ui->tableValues->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->tableValues->setEditTriggers(QTableView::NoEditTriggers);
  ui->tableValues->horizontalHeader()->setStretchLastSection(true);
  ui->tableValues->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

}

ViewRecord::~ViewRecord()
{
  delete ui;
}


void ViewRecord::set_trim(bool trim)
{
  trim_ = trim;
  display_current_record();
}

void ViewRecord::set_show_raw(bool show_raw)
{
  show_raw_ = show_raw;
  display_current_record();
}

void ViewRecord::set_overlay_type(QString overlay_type)
{
  overlay_type_ = overlay_type;
  display_current_record();
}

void ViewRecord::set_projection_type(QString projection_type)
{
  projection_type_ = projection_type;
  display_current_record();
}

void ViewRecord::set_dimenstions(NMX::Dimensions dim)
{
  dim_ = dim;
  display_current_record();
}

void ViewRecord::clear()
{
  ui->plotRecord->reset_content();
  ui->plotRecord->set_boxes(std::list<MarkerBox2D>());
  ui->plotRecord->refresh();

  moving_.visible = false;
  ui->plotProjection->reset_scales();
  ui->plotProjection->clearGraphs();
  ui->plotProjection->clearExtras();
  ui->plotProjection->replot_markers();
  ui->plotProjection->rescale();
  ui->plotProjection->redraw();
}


void ViewRecord::display_record(const NMX::Record &record)
{
  record_ = record;
  display_current_record();
}

void ViewRecord::display_current_record()
{
  clear();

  dim_shifted_ = dim_;
  if (trim_)
  {
    dim_shifted_.min = dim_.transform(record_.strip_start()-0.5);
    dim_shifted_.max = dim_.transform(record_.strip_end()+0.5);
    dim_shifted_.strips = record_.strip_end() - record_.strip_start() + 1;
  }

  ui->plotRecord->set_axes("Position (mm)", dim_shifted_.transform(0), dim_shifted_.transform(dim_shifted_.strips-1),
                           "Time bin",                             0, record_.time_end(),
                           "Charge");

  if (show_raw_)
    ui->plotRecord->update_plot(dim_shifted_.strips, record_.time_end() + 1, make_list());
  else
    ui->plotRecord->update_plot(dim_shifted_.strips, record_.time_end() + 1, EntryList());


  auto overlay = make_overlay();
  int stripi = record_.get_value("entry_strip");
  if (stripi >= 0)
  {
    if (trim_)
      stripi -= record_.strip_start();

    int timei = record_.get_value("entry_time");

    MarkerBox2D box;
    box.mark_center = true;
    box.x_c = dim_shifted_.transform(stripi);
    box.x1 = dim_shifted_.transform(stripi - 0.2);
    box.x2 = dim_shifted_.transform(stripi + 0.2);
    box.y_c = timei;
    box.y1  = timei - 0.2;
    box.y2  = timei + 0.2;
    overlay.push_back(box);
  }
  ui->plotRecord->set_boxes(overlay);

  ui->plotRecord->replot_markers();

  ui->tableValues->clearContents();
  auto valnames = record_.categories();
  ui->tableValues->setRowCount(valnames.size());
  int i = 0;
  for (auto &name : valnames)
  {
    add_to_table(ui->tableValues, i, 0, name);
    add_to_table(ui->tableValues, i, 1, std::to_string(record_.get_value(name)));
    i++;
  }

  display_projection();
}

EntryList ViewRecord::make_list()
{
  EntryList data;

  for (auto &i : record_.valid_strips())
  {
    auto strip = record_.get_strip(i);
    int stripi = i;
    if (trim_)
      stripi -= record_.strip_start();
    for (int tb=strip.bin_start(); tb <= strip.bin_end(); ++tb)
      if (strip.value(tb))
        data.push_back(Entry{{stripi,tb}, strip.value(tb)});
  }
  return data;
}

std::list<MarkerBox2D> ViewRecord::make_overlay()
{
  std::list<MarkerBox2D> ret;

  for (auto &i : record_.get_points(overlay_type_.toStdString()))
  {
    int stripi = i.first;
    if (trim_)
      stripi -= record_.strip_start();

    MarkerBox2D box;
    box.x_c = dim_shifted_.transform(stripi);
    box.x1 = dim_shifted_.transform(stripi - 0.45);
    box.x2 = dim_shifted_.transform(stripi + 0.45);
    box.y_c = i.second;
    box.y1 = i.second - 0.45;
    box.y2 = i.second + 0.45;
    ret.push_back(box);
  }

  return ret;
}


void ViewRecord::display_projection()
{
  std::map<double, double> minima, maxima;
  Calibration calib_ = Calibration();

  ui->plotProjection->clearGraphs();

  QVector<double> x, y;

  if (record_.valid_strips().size() && (projection_type_ != "none"))
  {
    for (size_t i = record_.strip_start(); i <= record_.strip_end(); ++i)
    {
      auto strip = record_.get_strip(i);
      double xx = dim_.transform(i);
      double yy = 0;

      if (projection_type_ == "Integral")
        yy = strip.integral();
//      else if (codomain == "Integral/bins")
//        yy = strip.integral_normalized();

      x.push_back(xx);
      y.push_back(yy);

      if (!minima.count(xx) || (minima[xx] > yy))
        minima[xx] = yy;
      if (!maxima.count(xx) || (maxima[xx] < yy))
        maxima[xx] = yy;
    }
  }

  AppearanceProfile profile;
  profile.default_pen = QPen(Qt::darkRed, 2);
  ui->plotProjection->addGraph(x, y, profile, 8);

  ui->plotProjection->use_calibrated(/*calib_.valid()*/false);
  ui->plotProjection->setLabels("position", "value");
  ui->plotProjection->setYBounds(minima, maxima);

  ui->plotProjection->setTitle(projection_type_);
  ui->plotProjection->replot_markers();
  ui->plotProjection->redraw();
}

