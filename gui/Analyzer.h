#ifndef ANALYZER_H
#define ANALYZER_H

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "FileHDF5.h"

#include "BoxesModel.h"
#include "SpecialDelegate.h"

#include "widget_plot2d.h"
#include "widget_plot_multi1d.h"

namespace Ui {
class Analyzer;
}

class Analyzer : public QWidget
{
  Q_OBJECT

public:
  explicit Analyzer(QWidget *parent = 0);
  ~Analyzer();

  void set_new_source(std::shared_ptr<NMX::FileHDF5> r);

public slots:
  void enableIO(bool);

signals:
  void toggleIO(bool);
  void select_indices(std::set<size_t>);

private slots:

  void plot_boxes();
  void parameters_set();

  void on_comboWeightsX_currentIndexChanged(const QString &arg1);
  void on_comboWeightsY_currentIndexChanged(const QString &arg1);
  void on_comboWeightsZ_currentIndexChanged(const QString &arg1);

  void on_spinMin_editingFinished();
  void on_spinMax_editingFinished();

  void on_pushAddBox_clicked();
  void update_box(double x, double y, bool left_mouse);

  void on_spinMin_valueChanged(int arg1);
  void on_spinMax_valueChanged(int arg1);

  void on_pushRemoveBox_clicked();

private:
  Ui::Analyzer *ui;

  std::shared_ptr<NMX::FileHDF5> reader_;


  QVector<Histogram> histograms1d_;
  BoxesModel model_;
  SpecialDelegate delegate_;

  std::map<int,std::map<std::pair<int,int>, std::list<size_t>>> data_;

  void loadSettings();
  void saveSettings();

  void rebuild_data();

  void make_projections();
  void update_histograms();
  void plot_block();

  double normalizer(const std::vector<double> &data);

  QVector<QColor> palette_ {Qt::black, Qt::darkRed, Qt::darkGreen, Qt::darkCyan,
        Qt::darkYellow, Qt::darkMagenta, Qt::darkBlue, Qt::red, Qt::blue};
};

#endif // FORM_CALIBRATION_H
