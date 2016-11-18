#ifndef BOXES_MODEL_H_
#define BOXES_MODEL_H_

#include <QDialog>
#include <QAbstractTableModel>
#include <QItemSelectionModel>

#include "Subhist.h"
#include "qt_util.h"


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
