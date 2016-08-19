#ifndef EVENT_VIEWER_H
#define EVENT_VIEWER_H

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "Reader.h"
#include "Dimensions.h"

#include "widget_plot2d.h"

namespace Ui {
class EventViewer;
}

class EventViewer : public QWidget
{
  Q_OBJECT

public:
  explicit EventViewer(QWidget *parent = 0);
  ~EventViewer();

  void set_new_source(std::shared_ptr<NMX::Reader> r, NMX::Dimensions x, NMX::Dimensions y);

  void clear();

public slots:
  void enableIO(bool);

private slots:
  void on_spinEventIdx_valueChanged(int arg1);
  void on_checkTrim_clicked();
  void on_checkNoneg_clicked();

  void on_comboOverlay_currentIndexChanged(const QString &arg1);

  void on_comboProjection_activated(const QString &arg1);

  void on_checkRaw_clicked();

private:
  Ui::EventViewer *ui;

  std::shared_ptr<NMX::Reader> reader_;

  NMX::Dimensions xdims_;
  NMX::Dimensions ydims_;

  void loadSettings();
  void saveSettings();
  void plot_current_event();

};

#endif // FORM_CALIBRATION_H
