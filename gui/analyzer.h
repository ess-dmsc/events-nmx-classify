#ifndef ANALYZER_H
#define ANALYZER_H

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "FileHDF5.h"
#include "Dimensions.h"

#include "widget_plot2d.h"
#include "widget_plot_multi1d.h"

namespace Ui {
class Analyzer;
}

struct HistParams
{
  int x1 {std::numeric_limits<int>::min()},
      x2{std::numeric_limits<int>::max()};
  int y1 {std::numeric_limits<int>::min()},
      y2{std::numeric_limits<int>::max()};
  double cutoff {0};
};

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
  void clear();

public slots:
  void enableIO(bool);

signals:
  void toggleIO(bool);

private slots:

  void update_box(double x, double y);

  void on_comboWeights_currentIndexChanged(const QString &arg1);

  void on_doubleNormalize_editingFinished();


  void on_spinBoxX_valueChanged(int arg1);
  void on_spinBoxY_valueChanged(int arg1);
  void on_spinBoxWidth_valueChanged(int arg1);
  void on_spinBoxHeight_valueChanged(int arg1);

  void on_spinMin_valueChanged(int arg1);
  void on_spinMax_valueChanged(int arg1);

private:
  Ui::Analyzer *ui;

  std::shared_ptr<NMX::FileHDF5> reader_;
  NMX::Dimensions xdims_;
  NMX::Dimensions ydims_;

  Marker1D marker_;


  QVector<HistParams> subset_params_;

  std::map<int,std::map<std::pair<int,int>, double>> data_;

  void loadSettings();
  void saveSettings();

  void rebuild_data();

  void make_projections();
  void update_histograms(const MultiHists&);

  void update_gates();

};

#endif // FORM_CALIBRATION_H
