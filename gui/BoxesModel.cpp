#include "BoxesModel.h"
#include <QFileDialog>
#include <QMessageBox>

#include "CustomLogger.h"


void HistParams::set_width(int w)
{
  auto center = center_x();
  x1 = center - w / 2;
  x2 = center + w / 2;
}

void HistParams::set_height(int h)
{
  auto center = center_y();
  y1 = center - h / 2;
  y2 = center + h / 2;
}

void HistParams::set_center_x(int x)
{
  auto w = width();
  x1 = x - w/2;
  x2 = x + w/2;
}

void HistParams::set_center_y(int y)
{
  auto h = height();
  y1 = y - h/2;
  y2 = y + h/2;
}



BoxesModel::BoxesModel(QVector<HistParams> &params, QObject *parent)
  : QAbstractTableModel(parent)
  , parameters_(params)
{
}

int BoxesModel::rowCount(const QModelIndex & /*parent*/) const
{
  return parameters_.size();
}

int BoxesModel::columnCount(const QModelIndex & /*parent*/) const
{
  return 6;
}

QVariant BoxesModel::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();

  if (role == Qt::DisplayRole)
  {
    switch (col) {
    case 0:
      return QVariant::fromValue(parameters_.at(row).color);
    case 1:
      return QVariant::fromValue(parameters_[row].visible);
    case 2:
      return QVariant::fromValue(parameters_.at(row).center_x());
    case 3:
      return QVariant::fromValue(parameters_.at(row).center_y());
    case 4:
      return QVariant::fromValue(parameters_.at(row).width());
    case 5:
      return QVariant::fromValue(parameters_.at(row).height());
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
      }
    } else if (orientation == Qt::Vertical) {
      return QString::number(section);
    }
  }
  return QVariant();
}

bool BoxesModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  setDataQuietly(index, value, role);
  emit editing_finished();
}


bool BoxesModel::setDataQuietly(const QModelIndex & index, const QVariant & value, int role)
{
  int row = index.row();
  int col = index.column();

  if ((role == Qt::EditRole) && (row >= 0) && (row < rowCount())
      && (col >= 0) && (col < columnCount()))
  {
    if (value.canConvert(QMetaType::LongLong))
    {
      if (col == 2)
        parameters_[row].set_center_x(value.toLongLong());
      else if (col == 3)
        parameters_[row].set_center_y(value.toLongLong());
      else if (col == 4)
        parameters_[row].set_width(value.toLongLong());
      else if (col == 5)
        parameters_[row].set_height(value.toLongLong());
      else
        return false;
    }
    else if ((value.type() == QVariant::Bool) && (col == 1))
      parameters_[row].visible = value.toBool();
    else if ((value.type() == QVariant::String) && (col == 0))
    {
      DBG << "model changing color " << value.toString().toStdString();
      parameters_[row].color = QColor(value.toString());
    }

    emit data_changed();
    return true;
  }

  return false;
}

void BoxesModel::update() {
  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex(parameters_.size(), columnCount());
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

Qt::ItemFlags BoxesModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags myflags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | QAbstractTableModel::flags(index);
  if (index.column() != 0)
    myflags |= Qt::ItemIsEditable;
  return myflags;
}

