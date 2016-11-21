#ifndef BOXES_MODEL_H_
#define BOXES_MODEL_H_

#include <QDialog>
#include <QAbstractTableModel>
#include <QItemSelectionModel>

#include "Subhist.h"
#include "qt_util.h"

struct MetricTest
{
  bool validate(double val) const { return (min <= val) && (val <= max); }

  bool enabled {false};
  std::string metric;
  double min {std::numeric_limits<double>::min()};
  double max {std::numeric_limits<double>::max()};
};


class TestsModel : public QAbstractTableModel
{
  Q_OBJECT

private:
  QVector<MetricTest> &tests_;
  std::list<std::string> available_metrics_;

signals:
  void data_changed();
  void editing_finished();

public:
  TestsModel(QVector<MetricTest> &, QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex & index) const;

  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

  void set_available_metrics(std::list<std::string> list) { available_metrics_ = list; }

public slots:
  bool setDataQuietly(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

  void update();
};



class BoxesModel : public QAbstractTableModel
{
  Q_OBJECT

private:
  QVector<Histogram> &slabs_;

signals:
  void data_changed();
  void editing_finished();

public:
  BoxesModel(QVector<Histogram> &, QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex & index) const;

  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

public slots:
  bool setDataQuietly(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

  void update();
};


#endif
