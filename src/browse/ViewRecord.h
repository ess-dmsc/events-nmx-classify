#pragma once

#include <QWidget>

#include "Plane.h"

#include "QPlot2D.h"
#include "QPlot1D.h"

namespace Ui {
class ViewRecord;
}

struct PointMetrics
{
  std::string x_metric;
  std::string y_metric;
  QColor color;
};

class ViewRecord : public QWidget
{
  Q_OBJECT

public:
  explicit ViewRecord(QWidget *parent = 0);
  ~ViewRecord();

  void set_title(QString);

  void clear();
  void display_record(const NMX::Plane &record, uint32_t width);
  void refresh();

  void set_plot_type(QString);
  void set_overlay_type(QString);
  void set_overlay_color(QColor);
  void set_point_metrics(QVector<PointMetrics>);

private:
  Ui::ViewRecord *ui;

  NMX::Plane record_;

  QPlot::Marker1D moving_;

  bool show_raw_{true};

  uint32_t width_{0};

  QString plot_type_;
  QString overlay_type_;
  QVector<PointMetrics> point_metrics_;

  QPlot::HistList2D make_list();
  QList<QPlot::MarkerBox2D> make_overlay();

  QColor overlay_color_ {Qt::red};

  QPlot::MarkerBox2D make_box(double cx, double cy, double size, QColor color);

};

