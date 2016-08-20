#include <QSettings>
#include "record_viewer.h"
#include "ui_record_viewer.h"
#include "qt_util.h"


RecordViewer::RecordViewer(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::RecordViewer)
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

  loadSettings();

}

RecordViewer::~RecordViewer()
{
  delete ui;
}

bool RecordViewer::save_close()
{
  saveSettings();
  return true;
}

void RecordViewer::loadSettings()
{
}

void RecordViewer::saveSettings()
{
}


void RecordViewer::clear()
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


void RecordViewer::display_record(const NMX::Record record, NMX::Dimensions dim, bool trim, bool raw, QString secondary)
{
  clear();

  NMX::Dimensions xdim = dim;
  if (trim)
  {
    xdim.min = dim.transform(record.strip_start()-0.5);
    xdim.max = dim.transform(record.strip_end()+0.5);
    xdim.strips = record.strip_end() - record.strip_start() + 1;
  }

  ui->plotRecord->set_axes("Position (mm)", xdim.transform(0), xdim.transform(xdim.strips-1),
                           "Time bin",                      0,    record.time_end(),
                           "Charge");

  if (raw)
    ui->plotRecord->update_plot(xdim.strips, record.time_end() + 1, make_list(record, trim));
  else
    ui->plotRecord->update_plot(xdim.strips, record.time_end() + 1, std::make_shared<EntryList>());

  if (secondary == "Maxima")
    ui->plotRecord->set_boxes(maxima(record, xdim, trim));
  else if (secondary == "VMM")
    ui->plotRecord->set_boxes(VMM(record, xdim, trim));

  ui->plotRecord->replot_markers();

  ui->tableValues->clearContents();
  auto valnames = record.categories();
  ui->tableValues->setRowCount(valnames.size());
  int i = 0;
  for (auto &name : valnames)
  {
    add_to_table(ui->tableValues, i, 0, name);
    add_to_table(ui->tableValues, i, 1, std::to_string(record.get_value(name)));
    i++;
  }
}

std::shared_ptr<EntryList> RecordViewer::make_list(const NMX::Record &record, bool trim)
{
  std::shared_ptr<EntryList> data = std::make_shared<EntryList>();

  for (auto &i : record.valid_strips())
  {
    auto strip = record.get_strip(i);
    int stripi = i;
    if (trim)
      stripi -= record.strip_start();
    for (int tb=strip.bin_start(); tb <= strip.bin_end(); ++tb)
      if (strip.value(tb))
        data->push_back(Entry{{stripi,tb}, strip.value(tb)});
  }
  return data;
}

std::list<MarkerBox2D> RecordViewer::maxima(const NMX::Record &record, NMX::Dimensions dim, bool trim)
{
  std::list<MarkerBox2D> ret;

  for (auto &i : record.valid_strips())
  {
    auto strip = record.get_strip(i);
    int stripi = i;
    if (trim)
      stripi -= record.strip_start();
    for (auto m : strip.maxima())
    {
      MarkerBox2D box;
      box.x_c = dim.transform(stripi);
      box.x1 = dim.transform(stripi - 0.45);
      box.x2 = dim.transform(stripi + 0.45);
      box.y_c = m;
      box.y1 = m - 0.45;
      box.y2 = m + 0.45;
      ret.push_back(box);
    }
    for (auto m : strip.global_maxima())
    {
      MarkerBox2D box;
      box.x_c = dim.transform(stripi);
      box.x1 = dim.transform(stripi - 0.2);
      box.x2 = dim.transform(stripi + 0.2);
      box.y_c = m;
      box.y1 = m - 0.2;
      box.y2 = m + 0.2;
      ret.push_back(box);
    }
  }

  return ret;
}


std::list<MarkerBox2D> RecordViewer::VMM(const NMX::Record &record, NMX::Dimensions dim, bool trim)
{
  std::list<MarkerBox2D> ret;

  for (auto idx : record.valid_strips())
  {
    int stripi = idx;
    if (trim)
      stripi -= record.strip_start();

    MarkerBox2D box;
    box.x_c = dim.transform(stripi);
    box.x1 = dim.transform(stripi - 0.45);
    box.x2 = dim.transform(stripi + 0.45);

    for (auto tb : record.get_strip(idx).VMM_maxima())
    {
      box.y_c = tb;
      box.y1  = tb - 0.45;
      box.y2  = tb + 0.45;
      ret.push_back(box);
    }
  }

  int stripi = record.get_value("entry_strip");
  if (stripi >= 0)
  {
    if (trim)
      stripi -= record.strip_start();

    int timei = record.get_value("entry_time");

    MarkerBox2D box;
    //  box.mark_center = true;
    box.x_c = dim.transform(stripi);
    box.x1 = dim.transform(stripi - 0.2);
    box.x2 = dim.transform(stripi + 0.2);
    box.y_c = timei;
    box.y1  = timei - 0.2;
    box.y2  = timei + 0.2;
    ret.push_back(box);
  }
  return ret;
}

void RecordViewer::display_projection(const NMX::Record &record, NMX::Dimensions dim, QString codomain)
{
  std::map<double, double> minima, maxima;
  Calibration calib_ = Calibration();

  ui->plotProjection->clearGraphs();

  QVector<double> x, y;

  if (record.valid_strips().size() && (codomain != "none"))
  {
    for (size_t i = record.strip_start(); i <= record.strip_end(); ++i)
    {
      auto strip = record.get_strip(i);
      double xx = dim.transform(i);
      double yy = 0;

      if (codomain == "Integral")
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

  ui->plotProjection->setTitle(codomain);
  ui->plotProjection->replot_markers();
  ui->plotProjection->redraw();
}

