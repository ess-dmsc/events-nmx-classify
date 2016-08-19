#ifndef ANALYZER_H
#define ANALYZER_H

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "Reader.h"
#include "Dimensions.h"

#include "widget_plot2d.h"
#include "widget_plot_multi1d.h"

#include "analysis_thread.h"

namespace Ui {
class Analyzer;
}

class Analyzer : public QWidget
{
  Q_OBJECT

public:
  explicit Analyzer(QWidget *parent = 0);
  ~Analyzer();

  void set_new_source(std::shared_ptr<NMX::Reader> r, NMX::Dimensions x, NMX::Dimensions y);
  void clear();

public slots:
  void enableIO(bool);

signals:
  void toggleIO(bool);

private slots:
  void on_pushStart_clicked();
  void on_pushStop_clicked();

  void update_data(std::shared_ptr<EntryList> data, double percent_done);
  void update_histograms(std::shared_ptr<MultiHists>);

  void run_complete();

  void on_spinMin_editingFinished();

  void on_spinMax_editingFinished();

  void update_box(double x, double y);
  void update_gates();

private:
  Ui::Analyzer *ui;

  std::shared_ptr<NMX::Reader> reader_;

  NMX::Dimensions xdims_;
  NMX::Dimensions ydims_;

  int box_x_{0}, box_y_{0};

  Marker1D marker_;

  AnalysisThread thread_;

  void loadSettings();
  void saveSettings();

  void resetPlot2D();

};

#endif // FORM_CALIBRATION_H
