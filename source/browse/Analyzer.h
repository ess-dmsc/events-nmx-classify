#ifndef ANALYZER_H
#define ANALYZER_H

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "File.h"

#include "BoxesModel.h"
#include "SpecialDelegate.h"


namespace Ui {
class Analyzer;
}

class Analyzer : public QWidget
{
  Q_OBJECT

public:
  explicit Analyzer(QWidget *parent = 0);
  ~Analyzer();

  void set_new_source(std::shared_ptr<NMX::File> r);
  void set_metric_z(QString);

signals:
  void select_indices(std::set<size_t>);

private slots:

  void on_pushAddTest_clicked();
  void on_pushRemoveTest_clicked();

  void on_comboFit_currentTextChanged(const QString &arg1);
  void on_doubleUnits_editingFinished();

  void rebuildFilteredList();
  void replot();

  void on_pushFilterFromPlot_clicked();

  void on_pushFilterToPlot_clicked();

  void filterSelectionChanged();

private:
  Ui::Analyzer *ui;
  TestsModel tests_model_;
  SpecialDelegate tests_delegate_;

  std::shared_ptr<NMX::File> reader_;
  std::set<size_t> indices_;
  HistMap1D histogram1d_;

  QVector<QColor> palette_ {Qt::black, Qt::darkRed, Qt::darkGreen,
                            Qt::darkYellow, Qt::darkMagenta,
                            Qt::red, Qt::blue,
                            Qt::darkCyan, Qt::darkBlue};

private:
  void loadSettings();
  void saveSettings();

  void replot1d();
  void populate_combos();

  QStringList getMetricsList(std::list<std::string> metric_set);
};

#endif
