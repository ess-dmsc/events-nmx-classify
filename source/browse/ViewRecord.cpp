#include <QSettings>
#include "ViewRecord.h"
#include "ui_ViewRecord.h"
#include "CustomLogger.h"

ViewRecord::ViewRecord(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ViewRecord)
{
  ui->setupUi(this);

  ui->plotRecord->setAntialiased(false);
  ui->plotRecord->setScaleType("Linear");
  ui->plotRecord->setGradient("YlGnBu5");
  ui->plotRecord->setShowGradientLegend(true);
  ui->plotRecord->setOrientation(Qt::Horizontal);
}

ViewRecord::~ViewRecord()
{
  delete ui;
}

void ViewRecord::set_title(QString title)
{
  ui->labelTitle->setText(title);
}

void ViewRecord::set_plot_type(QString plot_type)
{
  plot_type_ = plot_type;
  display_current_record();
}

void ViewRecord::set_overlay_type(QString overlay_type)
{
  overlay_type_ = overlay_type;
  display_current_record();
}

void ViewRecord::set_point_type1x(QString point_type)
{
  point_type1x_ = point_type;
  display_current_record();
}

void ViewRecord::set_point_type2x(QString point_type)
{
  point_type2x_ = point_type;
  display_current_record();
}

void ViewRecord::set_point_type1y(QString point_type)
{
  point_type1y_ = point_type;
  display_current_record();
}

void ViewRecord::set_point_type2y(QString point_type)
{
  point_type2y_ = point_type;
  display_current_record();
}

void ViewRecord::clear()
{
  ui->plotRecord->clearAll();
  ui->plotRecord->replot();
  moving_.visible = false;
}


void ViewRecord::display_record(const NMX::Record &record)
{
  record_ = record;
  display_current_record();
}

void ViewRecord::display_current_record()
{
  clear();

  ui->plotRecord->setAxes("Position (strip)", record_.strip_start(), record_.strip_end(),
                          "Time bin",                          0, record_.time_end(),
                          "Charge");

  if (plot_type_ != "none")
    ui->plotRecord->updatePlot(record_.strip_span(), record_.time_end() + 1, make_list());

  auto metrics = record_.metrics();

  auto overlay = make_overlay();

  double strip1 = metrics.get_value(point_type1x_.toStdString()).as_float(-1);
  double time1 = metrics.get_value(point_type1y_.toStdString()).as_float(-1);
  if ((strip1 >= 0.0) && (strip1 >= record_.strip_start()) && (strip1 <= record_.strip_end())
      && (time1 >= 0.0) && (time1 >= record_.time_start()) && (time1 <= record_.time_end()))
    overlay.push_back(make_box(strip1, time1, 0.5, Qt::yellow));

  double strip2 = metrics.get_value(point_type2x_.toStdString()).as_float(-1);
  double time2 = metrics.get_value(point_type2y_.toStdString()).as_float(-1);
  if ((strip2 >= 0.0) && (strip2 >= record_.strip_start()) && (strip2 <= record_.strip_end())
    && (time2 >= 0.0) && (time2 >= record_.time_start()) && (time2 <= record_.time_end()))
    overlay.push_back(make_box(strip2, time2, 0.4, Qt::magenta));

  ui->plotRecord->setBoxes(overlay);
  ui->plotRecord->replotExtras();
  ui->plotRecord->zoomOut();
}

HistList2D ViewRecord::make_list()
{
  HistList2D data;
  auto start = record_.strip_start();
  for (auto &p : record_.get_points(plot_type_.toStdString()))
    data.push_back(p2d{p.x - start, p.y, p.v});
  return data;
}

QPlot::MarkerBox2D ViewRecord::make_box(double cx, double cy, double size, QColor color)
{
  QPlot::MarkerBox2D box;
  box.x1 = cx - 0.5 *size;
  box.x2 = cx + 0.5 *size;
  box.y1  = cy - 0.5 *size;
  box.y2  = cy + 0.5 *size;
  box.border = color;
  QColor color2 = QColor::fromHsvF(color.hsvHueF(), color.hsvSaturationF(), color.valueF() * 0.50);
  color2.setAlpha(90);
  box.fill = color2;
  return box;
}


std::list<QPlot::MarkerBox2D> ViewRecord::make_overlay()
{
  std::list<QPlot::MarkerBox2D> ret;

  if (overlay_type_ == "none")
    return ret;

  for (auto &i : record_.get_points(overlay_type_.toStdString()))
  {
    auto box = make_box(i.x, i.y, 0.9, Qt::red);
    box.fill.setAlpha(48);
    ret.push_back(box);
  }

  return ret;
}




