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
class ViewRecord;
}

class ViewRecord : public QWidget
{
  Q_OBJECT

public:
  explicit ViewRecord(QWidget *parent = 0);
  ~ViewRecord();

  void display_record(const NMX::Record record, NMX::Dimensions dim, bool trim, bool raw, QString secondary);
  void display_projection(const NMX::Record &record, NMX::Dimensions dim, QString codomain);

  bool save_close();
  void clear();

private:
  Ui::ViewRecord *ui;

  Marker1D moving_;

  void loadSettings();
  void saveSettings();

  std::shared_ptr<EntryList> make_list(const NMX::Record &record, bool trim);

  std::list<MarkerBox2D> make_overlay(std::string type, const NMX::Record &record, NMX::Dimensions dim, bool trim);
};

#endif // FORM_CALIBRATION_H
