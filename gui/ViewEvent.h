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

  void set_params(std::map<std::string, double>);

  void clear();

public slots:
  void enableIO(bool);

  void set_indices(std::set<size_t> indices);

private slots:
  void on_spinEventIdx_valueChanged(int arg1);
  void on_checkNoneg_clicked();

  void on_comboOverlay_currentIndexChanged(const QString &arg1);

  void on_comboProjection_activated(const QString &arg1);

  void on_checkRaw_clicked();

private:
  Ui::ViewEvent *ui;

  std::shared_ptr<NMX::FileHDF5> reader_;
  std::map<std::string, double> params_;

  std::vector<size_t> indices_;

  void loadSettings();
  void saveSettings();
  void plot_current_event();

};

#endif // FORM_CALIBRATION_H
