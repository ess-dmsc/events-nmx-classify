#ifndef RECORD_VIEWER_H
#define RECORD_VIEWER_H

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "Record.h"
#include "Dimensions.h"

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

  void display_record(const NMX::Record record, NMX::Dimensions dim, bool trim, bool raw, QString secondary);
  void display_projection(const NMX::Record &record, NMX::Dimensions dim, QString codomain);

  bool save_close();
  void clear();

private:
  Ui::RecordViewer *ui;

  Marker1D moving_;

  void loadSettings();
  void saveSettings();

  std::shared_ptr<EntryList> make_list(const NMX::Record &record, bool trim);

  std::list<MarkerBox2D> maxima(const NMX::Record &record, NMX::Dimensions dim, bool trim);
  std::list<MarkerBox2D> VMM(const NMX::Record &record, NMX::Dimensions dim, bool trim);

};

#endif // FORM_CALIBRATION_H
