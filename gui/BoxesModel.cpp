#include "BoxesModel.h"
#include <QFileDialog>
#include <QMessageBox>

#include "CustomLogger.h"


BoxesModel::BoxesModel(QVector<Histogram> &params, QObject *parent)
  : QAbstractTableModel(parent)
  , slabs_(params)
{
}

int BoxesModel::rowCount(const QModelIndex & /*parent*/) const
{
  return slabs_.size();
}

int BoxesModel::columnCount(const QModelIndex & /*parent*/) const
{
  return 13;
}

QVariant BoxesModel::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();

  if (role == Qt::ForegroundRole)
  {
    if ((col > 0) && (col < 8))
      return QBrush(Qt::black);
    else
      return QBrush(Qt::darkGray);
  }
  else if (role == Qt::DisplayRole)
  {
    switch (col) {
    case 0:
      return QVariant::fromValue(slabs_.at(row).color);
    case 1:
      return QVariant::fromValue(slabs_[row].visible);
    case 2:
      return QVariant::fromValue(slabs_.at(row).center_x());
    case 3:
      return QVariant::fromValue(slabs_.at(row).center_y());
    case 4:
      return QVariant::fromValue(slabs_.at(row).width());
    case 5:
      return QVariant::fromValue(slabs_.at(row).height());

    case 6:
      return QVariant::fromValue(slabs_.at(row).bin_min());
    case 7:
      return QVariant::fromValue(slabs_.at(row).bin_max());

    case 8:
      return QVariant::fromValue(slabs_.at(row).min());
    case 9:
      return QVariant::fromValue(slabs_.at(row).max());
    case 10:
      return QVariant::fromValue(slabs_.at(row).avg());
    case 11:
      return QVariant::fromValue(slabs_.at(row).total_count());

    }
  }
  return QVariant();
}

QVariant BoxesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal) {
      switch (section)
      {
      case 0:
        return QString("Color");
      case 1:
        return QString("Visible");
      case 2:
        return QString("x center");
      case 3:
        return QString("y center");
      case 4:
        return QString("width");
      case 5:
        return QString("height");

      case 6:
        return QString("bin_min");
      case 7:
        return QString("bin_max");


      case 8:
        return QString("min");
      case 9:
        return QString("max");
      case 10:
        return QString("weighted avg");
      case 11:
        return QString("total count");


      }
    } else if (orientation == Qt::Vertical) {
      return QString::number(section);
    }
  }
  return QVariant();
}

bool BoxesModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  bool ret = setDataQuietly(index, value, role);
  emit editing_finished();
  return ret;
}


bool BoxesModel::setDataQuietly(const QModelIndex & index, const QVariant & value, int role)
{
  int row = index.row();
  int col = index.column();

  if ((role == Qt::EditRole) && (row >= 0) && (row < rowCount())
      && (col >= 0) && (col < columnCount()))
  {
    if ((col == 0) && value.canConvert(QVariant::String))
    {
      DBG << "model changing color " << value.toString().toStdString();
      slabs_[row].color = QColor(value.toString());
    }
    else if ((col == 1) && value.canConvert(QVariant::Bool))
      slabs_[row].visible = value.toBool();
    else if (value.canConvert(QMetaType::LongLong))
    {
      if (col == 2)
        slabs_[row].set_center_x(value.toLongLong());
      else if (col == 3)
        slabs_[row].set_center_y(value.toLongLong());
      else if (col == 4)
        slabs_[row].set_width(value.toLongLong());
      else if (col == 5)
        slabs_[row].set_height(value.toLongLong());
      else if (col == 6)
        slabs_[row].set_bin_bounds(value.toLongLong(), slabs_[row].bin_max());
      else if (col == 7)
        slabs_[row].set_bin_bounds(slabs_[row].bin_min(), value.toLongLong());
      else
        return false;
    }


    emit data_changed();
    return true;
  }

  return false;
}

void BoxesModel::update()
{
  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex(slabs_.size(), columnCount());
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

Qt::ItemFlags BoxesModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags myflags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | QAbstractTableModel::flags(index);
  if ((index.column() > 0) && (index.column() < 8))
    myflags |= Qt::ItemIsEditable;
  return myflags;
}

