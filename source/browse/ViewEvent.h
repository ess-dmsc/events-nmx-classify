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
#include "hue_slider.hpp"
#include "color_preview.hpp"

#include <QLabel>

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

  void refresh_event();

  bool x_visible();
  bool y_visible();

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


  void metrics_selected();

  void table_changed();
  void display_params();

  void set_point_metrics();

  void on_pushShowParams_clicked();

  void on_push1x_clicked();
  void on_push1y_clicked();
  void on_push2x_clicked();
  void on_push2y_clicked();

  void clicked_color1();
  void clicked_color2();

  void clicked_colorOverlay();

private:
  Ui::ViewEvent *ui;

  std::shared_ptr<NMX::FileAPV> reader_;
  NMX::Event event_;

  std::vector<size_t> indices_;

  void loadSettings();
  void saveSettings();
  void plot_current_event();

  void display_projection(NMX::Event &evt);

  MetricsModel metrics_model_;

  TableSettings params_model_;
  SpecialDelegate params_delegate_;

  void populateCombos(const NMX::Settings &parameters);

  void make_coord_popup(QPushButton *button,
                        NMX::MetricSet metric_set);

  void pick_color(color_widgets::ColorPreview* prev);
};

#endif // FORM_CALIBRATION_H
