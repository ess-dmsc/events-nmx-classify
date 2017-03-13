#pragma once

#include <QWidget>
#include <QItemSelection>
#include <memory>

#include "qp_2d.h"
#include "qp_multi1d.h"
#include "histogram.h"

namespace Ui {
class AggregateReview;
}

class AggregateReview : public QWidget
{
  Q_OBJECT

public:
  explicit AggregateReview(QWidget *parent = 0);
  ~AggregateReview();

signals:
  void toggleIO(bool);

  void digDownTown(QString, QString, QString);

private slots:
  void on_pushOpen_clicked();
  void on_pushNormalize_clicked();
  void on_pushDigDown_clicked();
  void on_pushGenColors_clicked();
  void on_spinMaxHists_editingFinished();

  void legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*);

  void render_selection();

private:
  Ui::AggregateReview *ui;

  QVector<QPen> palette_;
  QString data_directory_;

  std::map<std::string, std::map<std::string, std::map<std::string, HistMap1D>>> data_;

  void loadSettings();
  void saveSettings();

  void openFile(QString fileName);
  void make_palette();
};
