#pragma once

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "File.h"

#include "BoxesModel.h"
#include "SpecialDelegate.h"

class Histogram1D
{
public:
  HistMap1D map() const;
  HistList1D list() const;

  void clear();
  void add_one(double bin);

  static std::list<std::string> values();
  double get_value(std::string) const;

  double mean() const;
  double harmonic_mean() const;
  double RMS() const;

  double midrange() const;
  double median() const;
  double mode() const;

private:
  HistMap1D map_;
  uint64_t count_ {0};
};


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

  void on_comboAverage_currentIndexChanged(int index);

  void on_pushSave1D_clicked();

  void on_pushSave2D_clicked();

private:
  Ui::Analyzer *ui;
  TestsModel tests_model_;
  SpecialDelegate tests_delegate_;

  std::shared_ptr<NMX::File> reader_;
  std::set<size_t> indices_;
  Histogram1D histogram1d_;

  HistMap2D histogram2d_;

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
