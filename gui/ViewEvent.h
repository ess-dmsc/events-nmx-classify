#ifndef EVENT_VIEWER_H
#define EVENT_VIEWER_H

#include <QWidget>
#include <memory>
#include <set>

#include "FileHDF5.h"


namespace Ui {
class ViewEvent;
}

class ViewEvent : public QWidget
{
  Q_OBJECT

public:
  explicit ViewEvent(QWidget *parent = 0);
  ~ViewEvent();

  void set_new_source(std::shared_ptr<NMX::FileHDF5> r);

  void set_params(NMX::Settings);

  void clear();

public slots:
  void enableIO(bool);

  void set_indices(std::set<size_t> indices);

private slots:
  void on_spinEventIdx_valueChanged(int arg1);

  void on_comboPlot_currentIndexChanged(const QString &arg1);
  void on_comboOverlay_currentIndexChanged(const QString &arg1);

  void on_comboProjection_activated(const QString &arg1);

  void on_comboPlanes_currentIndexChanged(const QString &arg1);

  void on_comboPoint1x_currentIndexChanged(const QString &arg1);
  void on_comboPoint2x_currentIndexChanged(const QString &arg1);

  void on_comboPoint1y_currentIndexChanged(const QString &arg1);
  void on_comboPoint2y_currentIndexChanged(const QString &arg1);

private:
  Ui::ViewEvent *ui;

  std::shared_ptr<NMX::FileHDF5> reader_;
  NMX::Settings params_;
  NMX::Event event_;

  std::vector<size_t> indices_;

  void loadSettings();
  void saveSettings();
  void plot_current_event();

  void display_projection(NMX::Event &evt);
};

#endif // FORM_CALIBRATION_H
