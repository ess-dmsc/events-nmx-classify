#include "TableSettings.h"
#include "SpecialDelegate.h"
#include <boost/algorithm/string.hpp>
#include <QDateTime>
#include "qt_util.h"
#include "CustomLogger.h"

#include "h5json.h"

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
    {
      nlohmann::json item = settings_.at(row)["value"];
      if (item.is_number_integer())
        return QVariant::fromValue(item.get<int64_t>());
      else if (item.is_number_unsigned())
        return QVariant::fromValue(item.get<uint64_t>());
      else if (item.is_number_float())
        return QVariant::fromValue(item.get<double>());
      else if (item.is_boolean())
        return QVariant::fromValue(item.get<bool>());
//      else if (item.count("___choice") && item.count("___options"))
//      {
//        H5CC::Enum<int16_t> menu = item;
//        return QString::fromStdString(menu.choice());
//      }
    }
    else
      return QVariant();
  }
  else if ((role == Qt::EditRole) && (col == 1))
    return QVariant::fromValue(settings_.at(row)["value"]);

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
  //int col = index.column();

  if (role == Qt::EditRole)
  {
    nlohmann::json item = settings_.at(row)["value"];

    if (item.is_number_integer() && value.canConvert(QMetaType::LongLong))
    {
      item = value.toLongLong();
    }
    else if (item.is_number_unsigned() && value.canConvert(QMetaType::ULongLong))
    {
      item = value.toULongLong();
    }
    else if (item.is_number_float() && value.canConvert(QMetaType::Double))
    {
      item = value.toDouble();
    }
    else if (item.is_boolean() && value.canConvert(QMetaType::Bool))
    {
      item = value.toBool();
    }
//    else if ((item.count("___options") || item.count("___choice")) &&
//             value.canConvert(QMetaType::QString))
//    {
//      H5CC::Enum<int16_t> menu = item;
//      menu.choose(value.toString().toStdString());
//      item = menu;
//    }
    else
      return false;

    settings_[row]["value"] = item;

    emit settings_changed();
    return true;
  }

  return true;
}

NMX::Settings TableSettings::get_settings() const
{
  NMX::Settings ret;
  for (int i=0; i < names_.size(); ++i)
    ret.set(names_.at(i).toStdString(),
            settings_.at(i)["value"],
            settings_.at(i)["description"]);
  return ret;
}

