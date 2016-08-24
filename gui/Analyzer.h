#ifndef ANALYZER_H
#define ANALYZER_H

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "FileHDF5.h"
#include "Dimensions.h"

#include "BoxesModel.h"
#include "SpecialDelegate.h"

#include "widget_plot2d.h"
#include "widget_plot_multi1d.h"

namespace Ui {
class Analyzer;
}

struct HistSubset
{
  double min{std::numeric_limits<int>::max()};
  double max{std::numeric_limits<int>::min()};
  double avg{0};
  double total_count{0};
  EntryList data;
};

using MultiHists = QVector<HistSubset>;

class Analyzer : public QWidget
{
  Q_OBJECT

public:
  explicit Analyzer(QWidget *parent = 0);
  ~Analyzer();

  void set_new_source(std::shared_ptr<NMX::FileHDF5> r, NMX::Dimensions x, NMX::Dimensions y);

public slots:
  void enableIO(bool);

signals:
  void toggleIO(bool);
  void select_indices(std::set<size_t>);

private slots:

  void parameters_changed();
  void parameters_set();

  void on_comboWeights_currentIndexChanged(const QString &arg1);
  void on_doubleNormalize_editingFinished();

  void on_spinMin_editingFinished();
  void on_spinMax_editingFinished();

  void on_pushAddBox_clicked();
  void update_box(double x, double y, bool left_mouse);

  void on_spinMin_valueChanged(int arg1);
  void on_spinMax_valueChanged(int arg1);

private:
  Ui::Analyzer *ui;

  std::shared_ptr<NMX::FileHDF5> reader_;
  NMX::Dimensions xdims_;
  NMX::Dimensions ydims_;


  QVector<HistParams> subset_params_;
  BoxesModel model_;
  SpecialDelegate delegate_;

  std::map<int,std::map<std::pair<int,int>, std::list<size_t>>> data_;

  void loadSettings();
  void saveSettings();

  void rebuild_data();

  void make_projections();
  void update_histograms(const MultiHists&);
  void plot_block();

  QVector<QColor> palette_ {Qt::black, Qt::darkRed, Qt::darkGreen, Qt::darkCyan, Qt::darkYellow, Qt::darkMagenta, Qt::darkBlue, Qt::red, Qt::blue};
};

#endif // FORM_CALIBRATION_H
