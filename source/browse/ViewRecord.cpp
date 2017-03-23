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

  ui->plotRecord->addCustomGradient("Tobias1", {"#ffffff",
                                                "#a1dab4", "#a1dab4",
                                                "#41b6c4", "#41b6c4", "#41b6c4",
                                                "#2c7fb8", "#2c7fb8", "#2c7fb8",
                                                "#253494", "#253494"}
                                      );


  ui->plotRecord->setGradient("Tobias1");
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
}

void ViewRecord::set_overlay_type(QString overlay_type)
{
  overlay_type_ = overlay_type;
}

void ViewRecord::set_point_metrics(QVector<PointMetrics> pm)
{
  point_metrics_ = pm;
}

void ViewRecord::set_overlay_color(QColor col)
{
  overlay_color_ = col;
}


void ViewRecord::clear()
{
  ui->plotRecord->clearAll();
  ui->plotRecord->replot();
  moving_.visible = false;
}


void ViewRecord::display_record(const NMX::Plane &record, uint32_t width)
{
  record_ = record;
  width_ = width;
  refresh();
}

void ViewRecord::refresh()
{
  clear();

  ui->plotRecord->setAxes("Position (strip)", record_.strip_start(), record_.strip_end(),
                          "Time bin",                          0, record_.time_end(),
                          "Charge");

  if (plot_type_ != "none")
    ui->plotRecord->updatePlot(record_.strip_span(), record_.time_end() + 1, make_list());

  auto metrics = record_.metrics();

  auto overlay = make_overlay();

  for (int i=0; i < point_metrics_.size(); ++i)
  {
    auto pm = point_metrics_.at(i);
    double strip1 = metrics.get_value(pm.x_metric);
    double time1 = metrics.get_value(pm.y_metric);
    if ((strip1 >= 0.0) && (strip1 >= record_.strip_start()) && (strip1 <= record_.strip_end())
        && (time1 >= 0.0) && (time1 >= record_.time_start()) && (time1 <= record_.time_end()))
      overlay.push_back(make_box(strip1, time1, 0.5, pm.color));
  }

  ui->plotRecord->setBoxes(overlay);
  ui->plotRecord->replotExtras();
  ui->plotRecord->zoomOut();
  if (width_)
  {
//    DBG << "Width= " << width_ << " instead of " << record_.strip_span();
    uint32_t center = record_.strip_start() + record_.strip_span() / 2;
    int32_t min = center - width_/2;
    int32_t max = center + width_/2;
    if (min < 0)
    {
      max += min;
      min = 0;
    }
    ui->plotRecord->xAxis->setRange(min, max);
    ui->plotRecord->replot();
  }
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
//  QColor color2 = QColor::fromHsvF(color.hsvHueF(),
//                                   color.hsvSaturationF(),
//                                   color.valueF() * 0.50);
//  color2.setAlpha(255);
  box.fill = color;
  return box;
}


std::list<QPlot::MarkerBox2D> ViewRecord::make_overlay()
{
  std::list<QPlot::MarkerBox2D> ret;

  if (overlay_type_ == "none")
    return ret;

  for (auto &i : record_.get_points(overlay_type_.toStdString()))
    ret.push_back(make_box(i.x, i.y, 0.9, overlay_color_));

  return ret;
}

