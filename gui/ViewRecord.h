#ifndef RECORD_VIEWER_H
#define RECORD_VIEWER_H

#include <QWidget>

#include "Record.h"

#include "widget_plot2d.h"
#include "widget_plot_multi1d.h"

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
  void set_suppress_negatives(bool noneg) { noneg_ = noneg; }

  void clear();

  void set_show_raw(bool);
  void set_overlay_type(QString);
  void set_title(QString);

private:
  Ui::ViewRecord *ui;

  NMX::Record record_;

  Marker1D moving_;

  bool show_raw_{true};
  bool noneg_{false};
  QString overlay_type_;

  void display_current_record();

  EntryList make_list();
  std::list<MarkerBox2D> make_overlay();
};

#endif // FORM_CALIBRATION_H
