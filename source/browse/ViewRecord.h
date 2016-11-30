#ifndef RECORD_VIEWER_H
#define RECORD_VIEWER_H

#include <QWidget>

#include "Record.h"

#include "qp_2d.h"
#include "qp_multi1d.h"

namespace Ui {
class ViewRecord;
}

class ViewRecord : public QWidget
{
  Q_OBJECT

public:
  explicit ViewRecord(QWidget *parent = 0);
  ~ViewRecord();

  void display_record(const NMX::Record &record);

  void clear();

  void set_plot_type(QString);
  void set_overlay_type(QString);

  void set_point_type1x(QString);
  void set_point_type2x(QString);

  void set_point_type1y(QString);
  void set_point_type2y(QString);

  void set_title(QString);

private:
  Ui::ViewRecord *ui;

  NMX::Record record_;

  QPlot::Marker1D moving_;

  bool show_raw_{true};

  QString plot_type_;
  QString overlay_type_;

  QString point_type1x_;
  QString point_type2x_;

  QString point_type1y_;
  QString point_type2y_;

  void display_current_record();

  HistList2D make_list();
  std::list<QPlot::MarkerBox2D> make_overlay();

  QPlot::MarkerBox2D make_box(double cx, double cy, double size, QColor color);
};

#endif // FORM_CALIBRATION_H