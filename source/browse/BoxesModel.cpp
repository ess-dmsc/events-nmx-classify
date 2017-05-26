#include "BoxesModel.h"
#include <QFileDialog>
#include <QMessageBox>

#include "CustomLogger.h"
#include "H5CC_Enum.h"
#include "SpecialDelegate.h"
#include "JsonH5.h"

TestsModel::TestsModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

int TestsModel::rowCount(const QModelIndex & /*parent*/) const
{
  return tests_.tests.size();
}

int TestsModel::columnCount(const QModelIndex & /*parent*/) const
{
  return 6;
}

QVariant TestsModel::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();

  if (role == Qt::ForegroundRole)
  {
    if ((col >= 0) && (col < 8))
      return QBrush(Qt::black);
    else
      return QBrush(Qt::darkGray);
  }
  else if ((role == Qt::EditRole) && (col == 1))
  {
    H5CC::Enum<int16_t> m(available_metrics_);
    m.choose(tests_.tests.at(row).metric);
    nlohmann::json j = m;
    return QVariant::fromValue(j);
  }
  else if (role == Qt::DisplayRole)
  {
    switch (col) {
    case 0:
      return QVariant::fromValue(tests_.tests.at(row).enabled);
    case 1:
      return QString::fromStdString(tests_.tests.at(row).metric);
    case 2:
      return QVariant::fromValue(tests_.tests.at(row).round_before_compare);
    case 3:
      return QVariant::fromValue(tests_.tests.at(row).min);
    case 4:
      return QVariant::fromValue(tests_.tests.at(row).max);
//    case 4:
//      return QString::fromStdString(tests_.tests.at(row).description);
    }
  }
  return QVariant();
}

QVariant TestsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal) {
      switch (section)
      {
      case 0:
        return QString("Enabled");
      case 1:
        return QString("Metric");
      case 2:
        return QString("Round intermediate");
      case 3:
        return QString("min");
      case 4:
        return QString("max");
      }
    } else if (orientation == Qt::Vertical) {
      return QString::number(section);
    }
  }
  return QVariant();
}

bool TestsModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  bool ret = setDataQuietly(index, value, role);
  emit editing_finished();
  return ret;
}


bool TestsModel::setDataQuietly(const QModelIndex & index,
                                const QVariant & value, int role)
{
  int row = index.row();
  int col = index.column();

  if ((role == Qt::EditRole) && (row >= 0) && (row < rowCount())
      && (col >= 0) && (col < columnCount()))
  {
    if ((col == 0) && value.canConvert(QVariant::Bool))
      tests_.tests[row].enabled = value.toBool();
    else if ((col == 1) && value.canConvert(QVariant::String))
    {
      tests_.tests[row].metric = value.toString().toStdString();
      update();
    }
    else if ((col == 2) && value.canConvert(QVariant::Bool))
    {
      tests_.tests[row].round_before_compare = value.toBool();
      update();
    }
    else if (value.canConvert(QMetaType::Double))
    {
      if (col == 3)
        tests_.tests[row].min = value.toDouble();
      else if (col == 4)
        tests_.tests[row].max = value.toDouble();
      else
        return false;
    }

    emit data_changed();
    return true;
  }

  return false;
}

void TestsModel::update()
{
  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex(tests_.tests.size(), columnCount());
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

Qt::ItemFlags TestsModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags myflags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | QAbstractTableModel::flags(index);
  if ((index.column() >= 0) && (index.column() < 5))
    myflags |= Qt::ItemIsEditable;
  return myflags;
}




MetricsModel::MetricsModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

void MetricsModel::set_metrics(NMX::MetricSet metrics)
{
  names_.clear();
  values_.clear();
  for (auto m : metrics.data())
  {
    names_.push_back(QString::fromStdString(m.first));
    values_.push_back(m.second.value);
  }
  update();
}

int MetricsModel::rowCount(const QModelIndex & /*parent*/) const
{
  return names_.size();
}

int MetricsModel::columnCount(const QModelIndex & /*parent*/) const
{
  return 2;
}

QVariant MetricsModel::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();

  if (role == Qt::DisplayRole)
  {
    switch (col) {
    case 0:
      return names_.at(row);
    case 1:
      return QString::number(values_.at(row));
    }
  }
  return QVariant();
}

QVariant MetricsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal) {
      switch (section)
      {
      case 0:
        return QString("Metric");
      case 1:
        return QString("Value");
      }
    } else if (orientation == Qt::Vertical) {
      return QString::number(section);
    }
  }
  return QVariant();
}

void MetricsModel::update()
{
  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex(rowCount(), columnCount());
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

