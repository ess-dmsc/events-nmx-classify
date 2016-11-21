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

struct MetricFilter
{
  bool validate(const std::map<std::string, NMX::Metric>& metrics, size_t index) const
  {
    for (auto f : tests)
    {
      if (!f.enabled)
        continue;
      if (!metrics.count(f.metric))
        return false;
      const auto& metric = metrics.at(f.metric);
      if (index >= metric.const_data().size())
        return false;
      if (!f.validate(metric.const_data().at(index)))
        return false;
    }
    return true;
  }

  std::list<std::string> required_metrics() const
  {
    std::list<std::string> ret;
    for (auto t : tests)
      if (t.enabled)
        ret.push_back(t.metric);
    return ret;
  }

  QVector<MetricTest> tests;
};


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
  void parameters_set();

  void on_comboWeightsX_currentIndexChanged(const QString &arg1);
  void on_comboWeightsY_currentIndexChanged(const QString &arg1);
  void on_comboWeightsZ_currentIndexChanged(const QString &arg1);

  void on_pushAddBox_clicked();
  void on_pushRemoveBox_clicked();
  void update_box(double x, double y, Qt::MouseButton left_mouse);

  void on_pushAddTest_clicked();

  void on_pushRemoveTest_clicked();

  void rebuild_data();

private:
  Ui::Analyzer *ui;

  std::shared_ptr<NMX::FileAPV> reader_;

  QVector<Histogram> histograms1d_;
  BoxesModel boxes_model_;
  SpecialDelegate boxes_delegate_;

  MetricFilter tests_;
  TestsModel tests_model_;
  SpecialDelegate tests_delegate_;

  void loadSettings();
  void saveSettings();

  void update_histograms();
  void plot_block();

  void populate_combos();

  double xx_norm {1};
  double yy_norm {1};
  double zz_norm {1};

  QVector<QColor> palette_ {Qt::black, Qt::darkRed, Qt::darkGreen,
                            Qt::darkYellow, Qt::darkMagenta,
                            Qt::red, Qt::blue,
                            Qt::darkCyan, Qt::darkBlue};
};

#endif // FORM_CALIBRATION_H
