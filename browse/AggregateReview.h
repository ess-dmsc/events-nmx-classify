#ifndef AGGREGATE_REVIEW_H
#define AGGREGATE_REVIEW_H

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

public slots:
  void enableIO(bool);

signals:
  void toggleIO(bool);

private slots:
  void on_pushOpen_clicked();

  void render_selection();

  void on_spinMaxHists_editingFinished();

private:
  Ui::AggregateReview *ui;

  void loadSettings();
  void saveSettings();

  QVector<QColor> palette_ {Qt::black, Qt::darkRed, Qt::darkGreen,
                            Qt::darkYellow, Qt::darkMagenta,
                            Qt::red, Qt::blue,
                            Qt::darkCyan, Qt::darkBlue};


  std::map<std::string, std::map<std::string, std::map<std::string, HistMap1D>>> data_;


};

#endif
