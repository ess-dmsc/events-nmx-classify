#ifndef RECORD_VIEWER_H
#define RECORD_VIEWER_H

#include <QWidget>
#include <QItemSelection>
#include "tpcRecord.h"
#include <memory>

#include "widget_plot2d.h"
#include "widget_plot_multi1d.h"

namespace Ui {
class RecordViewer;
}

class RecordViewer : public QWidget
{
  Q_OBJECT

public:
  explicit RecordViewer(QWidget *parent = 0);
  ~RecordViewer();

  void display_record(const TPC::Record record, TPC::Dimensions dim, bool trim, QString secondary);
  void display_projection(const TPC::Record &record, TPC::Dimensions dim, QString codomain);

  bool save_close();
  void clear();

private:
  Ui::RecordViewer *ui;

  Marker1D moving_;

  void loadSettings();
  void saveSettings();

  std::shared_ptr<EntryList> make_list(const TPC::Record &record, bool trim);

  std::list<MarkerBox2D> maxima(const TPC::Record &record, TPC::Dimensions dim, bool trim);
  std::list<MarkerBox2D> VMMx(const TPC::Record &record, TPC::Dimensions dim, bool trim);

};

#endif // FORM_CALIBRATION_H
