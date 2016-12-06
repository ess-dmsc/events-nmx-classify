#include "BoxesModel.h"
#include <QFileDialog>
#include <QMessageBox>

#include "CustomLogger.h"
#include "MultipleChoice.h"
#include "Variant.h"
#include "SpecialDelegate.h"


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
  return 5;
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
    MultipleChoice m(available_metrics_);
    m.choose(tests_.tests.at(row).metric);
    return QVariant::fromValue(Variant::from_menu(m));
  }
  else if (role == Qt::DisplayRole)
  {
    switch (col) {
    case 0:
      return QVariant::fromValue(tests_.tests.at(row).enabled);
    case 1:
      return QString::fromStdString(tests_.tests.at(row).metric);
    case 2:
      return QVariant::fromValue(tests_.tests.at(row).min);
    case 3:
      return QVariant::fromValue(tests_.tests.at(row).max);
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
        return QString("min");
      case 3:
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


bool TestsModel::setDataQuietly(const QModelIndex & index, const QVariant & value, int role)
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
    else if (value.canConvert(QMetaType::LongLong))
    {
      if (col == 2)
        tests_.tests[row].min = value.toLongLong();
      else if (col == 3)
        tests_.tests[row].max = value.toLongLong();
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
  if ((index.column() >= 0) && (index.column() < 8))
    myflags |= Qt::ItemIsEditable;
  return myflags;
}

