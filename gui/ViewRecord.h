#ifndef RECORD_VIEWER_H
#define RECORD_VIEWER_H

#include <QWidget>

#include "Record.h"
#include "Dimensions.h"

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

  void clear();

  void set_trim(bool);
  void set_show_raw(bool);
  void set_overlay_type(QString);
  void set_projection_type(QString);
  void set_dimenstions(NMX::Dimensions);

private:
  Ui::ViewRecord *ui;

  NMX::Record record_;

  Marker1D moving_;

  bool trim_ {true};
  bool show_raw_{true};
  QString overlay_type_;
  QString projection_type_;
  NMX::Dimensions dim_;
  NMX::Dimensions dim_shifted_;


  void display_current_record();

  void display_projection();
  EntryList make_list();
  std::list<MarkerBox2D> make_overlay();
};

#endif // FORM_CALIBRATION_H
