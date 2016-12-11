#ifndef EVENT_VIEWER_H
#define EVENT_VIEWER_H

#include <QWidget>
#include <memory>
#include <set>

#include "FileAPV.h"
#include "TableSettings.h"
#include "SpecialDelegate.h"
#include "BoxesModel.h"
#include "Settings.h"

namespace Ui {
class ViewEvent;
}

class ViewEvent : public QWidget
{
  Q_OBJECT

public:
  explicit ViewEvent(QWidget *parent = 0);
  ~ViewEvent();

  void set_new_source(std::shared_ptr<NMX::FileAPV> r);
  void clear();

public slots:
  void enableIO(bool);
  void set_indices(std::set<size_t> indices);

private slots:
  void on_spinEventIdx_valueChanged(int arg1);

  //mostly visual
  void on_comboPlanes_currentIndexChanged(const QString &arg1);
  void on_pushShowParams_clicked();
  void metricsFilterChanged();
  void recordOptionsChanged();
  void plotProjection();

  //change analysis parameters
  void parametersModified();

private:
  Ui::ViewEvent *ui;

  std::shared_ptr<NMX::FileAPV> reader_;
  std::vector<size_t> indices_;

  NMX::Event event_;

  MetricsModel metrics_model_;

  TableSettings params_model_;
  SpecialDelegate params_delegate_;

private:
  void loadSettings();
  void saveSettings();
  void populateCombos();


  void plot_current_event();

  void set_record_options();
  void refresh_record_plots();
  void update_parameters_model();
};

#endif
