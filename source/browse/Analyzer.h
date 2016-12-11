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

#include "doFit.h"

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
  void set_metric_z(QString);

public slots:
  void enableIO(bool);

signals:
  void toggleIO(bool);
  void select_indices(std::set<size_t>);

private slots:

  void plot_boxes();

  void on_pushAddTest_clicked();
  void on_pushRemoveTest_clicked();

  void on_comboFit_currentTextChanged(const QString &arg1);

  void on_doubleUnits_editingFinished();

  void rebuild();

  void on_pushX_clicked();
  void on_pushY_clicked();
  void on_pushMetric1D_clicked();

private:
  Ui::Analyzer *ui;

  std::shared_ptr<NMX::FileAPV> reader_;

//  MetricFilter tests_;
  TestsModel tests_model_;
  SpecialDelegate tests_delegate_;

  void loadSettings();
  void saveSettings();

  void plot_block();

  void populate_combos();

  std::set<size_t> indices_;
  HistMap1D histogram1d_;

  QVector<QColor> palette_ {Qt::black, Qt::darkRed, Qt::darkGreen,
                            Qt::darkYellow, Qt::darkMagenta,
                            Qt::red, Qt::blue,
                            Qt::darkCyan, Qt::darkBlue};

  void replot();

  void replot1d();


  void make_coord_popup(QPushButton *button,
                        std::list<std::string> metric_set);
};

#endif // FORM_CALIBRATION_H
