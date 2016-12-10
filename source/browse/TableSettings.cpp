#include "TableSettings.h"
#include "SpecialDelegate.h"
#include <boost/algorithm/string.hpp>
#include <QDateTime>
#include "qt_util.h"
#include "CustomLogger.h"

TableSettings::TableSettings(QObject *parent)
  : QAbstractTableModel(parent)
{}


int TableSettings::rowCount(const QModelIndex & /*parent*/) const
{
  return settings_.size();
}

int TableSettings::columnCount(const QModelIndex & /*parent*/) const
{
  return 2;
}

QVariant TableSettings::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();

  if (role == Qt::DisplayRole)
  {
    if (col == 0)
      return names_.at(row);
    else if (col == 1)
      return QString::fromStdString(settings_.at(row).value.to_string());
    else
      return QVariant();
  }
  else if ((role == Qt::EditRole) && (col == 1))
    return QVariant::fromValue(settings_.at(row).value);

  return QVariant();
}

QVariant TableSettings::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal) {
      if (section == 0)
        return QString("Parameter");
      else if (section == 1)
        return QString("Value");
      else if (section == 2)
        return "Description";
    } else if (orientation == Qt::Vertical) {
      return QString::number(section);
    }
  }
  return QVariant();
}

void TableSettings::update(const NMX::Settings &settings) {
  settings_.clear();
  names_.clear();
  for (auto s : settings.data())
  {
    names_.push_back(QString::fromStdString(s.first));
    settings_.push_back(s.second);
  }

  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex( rowCount() - 1, columnCount() - 1);
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

Qt::ItemFlags TableSettings::flags(const QModelIndex &index) const
{
  int col = index.column();

  if (col == 1)
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
  else
    return Qt::ItemIsEnabled | QAbstractTableModel::flags(index);
}

bool TableSettings::setData(const QModelIndex & index, const QVariant & value, int role)
{
  int row = index.row();
  int col = index.column();

  if (role == Qt::EditRole)
  {
    auto item = settings_.at(row).value;

    if ((item.type() == Variant::code::type_int) && (value.canConvert(QMetaType::LongLong)))
    {
      auto val = item.as_int();
      val.set_val(value.toLongLong());
      item = Variant::from_int(val);
    }
    else if ((item.type() == Variant::code::type_uint) && (value.canConvert(QMetaType::ULongLong)))
    {
      auto val = item.as_uint();
      val.set_val(value.toULongLong());
      item = Variant::from_uint(val);
    }
    else if ((item.type() == Variant::code::type_float) && (value.canConvert(QMetaType::Double)))
    {
      auto val = item.as_float();
      val.set_val(value.toDouble());
      item = Variant::from_float(val);
    }
    else if ((item.type() == Variant::code::type_bool) && (value.canConvert(QMetaType::Bool)))
      item = Variant::from_bool(value.toBool());
    else
      return false;

    settings_[row].value = item;

    emit settings_changed();
    return true;
  }

  return true;
}

NMX::Settings TableSettings::get_settings() const
{
  NMX::Settings ret;
  for (int i=0; i < names_.size(); ++i)
    ret.set(names_.at(i).toStdString(), settings_.at(i));
  return ret;
}

