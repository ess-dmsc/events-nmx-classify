#ifndef ANALYZER_H
#define ANALYZER_H

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "FileAPV.h"

#include "BoxesModel.h"
#include "SpecialDelegate.h"

#include "qp_2d.h"
#include "qp_multi1d.h"

namespace Ui {
class Analyzer;
}

class Analyzer : public QWidget
{
  Q_OBJECT

public:
  explicit Analyzer(QWidget *parent = 0);
  ~Analyzer();

  void set_new_source(std::shared_ptr<NMX::FileAPV> r);

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

  void on_spinMinX_editingFinished();
  void on_spinMaxX_editingFinished();

  void on_spinMinY_editingFinished();
  void on_spinMaxY_editingFinished();

  void on_spinMinZ_editingFinished();
  void on_spinMaxZ_editingFinished();

  void on_pushAddBox_clicked();
  void update_box(double x, double y, Qt::MouseButton left_mouse);

  void on_spinMinZ_valueChanged(int arg1);
  void on_spinMaxZ_valueChanged(int arg1);

  void on_pushRemoveBox_clicked();

private:
  Ui::Analyzer *ui;

  std::shared_ptr<NMX::FileAPV> reader_;


  QVector<Histogram> histograms1d_;
  BoxesModel model_;
  SpecialDelegate delegate_;

  std::map<int,std::map<std::pair<int32_t,int32_t>, std::list<size_t>>> data_;

  void loadSettings();
  void saveSettings();

  void rebuild_data();

  void make_projections();
  void update_histograms();
  void plot_block();

  void populate_combos();

  double xx_norm {1};
  double yy_norm {1};
  double zz_norm {1};

  double normalizer(const std::vector<double> &data);

  QVector<QColor> palette_ {Qt::black, Qt::darkRed, Qt::darkGreen,
                            Qt::darkYellow, Qt::darkMagenta,
                            Qt::red, Qt::blue,
                            Qt::darkCyan, Qt::darkBlue};
};

#endif // FORM_CALIBRATION_H
