#pragma once

#include <QDialog>
#include <QAbstractTableModel>
#include <QItemSelectionModel>

#include "Subhist.h"
#include "qt_util.h"
#include "MetricSet.h"

class TestsModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  TestsModel(QObject *parent = 0);
  void set_available_metrics(std::list<std::string> list) { available_metrics_ = list; }
  void set_tests(MetricFilter tests) { tests_ = tests; update(); }
  MetricFilter tests() const { return tests_; }

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex & index) const;
  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

public slots:
  bool setDataQuietly(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
  void update();

signals:
  void data_changed();
  void editing_finished();

private:
  MetricFilter tests_;
  std::list<std::string> available_metrics_;
};



class MetricsModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  MetricsModel(QObject *parent = 0);
  void set_metrics(NMX::MetricSet metrics);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
  void update();

private:
  QVector<QString> names_;
  QVector<double> values_;
};


