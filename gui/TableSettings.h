#ifndef TABLE_SETTINGS_H_
#define TABLE_SETTINGS_H_

#include <QAbstractTableModel>
#include <QFont>
#include <QBrush>
#include "Settings.h"

class TableSettings : public QAbstractTableModel
{
    Q_OBJECT
private:
    QVector<NMX::Setting> settings_;
    QVector<QString> names_;

public:
    TableSettings(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    void update(const NMX::Settings &settings);

    NMX::Settings get_settings() const;

signals:
    void settings_changed();

};

#endif
