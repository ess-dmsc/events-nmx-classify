#ifndef ANALYZER_H
#define ANALYZER_H

#include <QWidget>
#include <QItemSelection>
#include "tpcReader.h"
#include <memory>

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

  void set_new_source(std::shared_ptr<TPC::Reader> r, TPC::Dimensions x, TPC::Dimensions y);
  void clear();

public slots:
  void enableIO(bool);

signals:
  void toggleIO(bool);

private slots:
  void on_pushStart_clicked();
  void on_pushStop_clicked();

  void update_data(std::shared_ptr<EntryList> data, double percent_done);
  void update_histogram(std::shared_ptr<EntryList> histo, QString codomain);
  void update_subhist(std::shared_ptr<EntryList> histo, QString codomain);

  void run_complete();

  void on_spinMin_editingFinished();

  void on_spinMax_editingFinished();

  void update_gates(double x, double y);

private:
  Ui::Analyzer *ui;

  std::shared_ptr<TPC::Reader> reader_;

  TPC::Dimensions xdims_;
  TPC::Dimensions ydims_;

  Marker1D marker_;

  AnalysisThread thread_;

  void loadSettings();
  void saveSettings();

  void resetPlot2D();

};

#endif // FORM_CALIBRATION_H
