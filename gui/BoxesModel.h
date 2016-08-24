#ifndef BOXES_MODEL_H_
#define BOXES_MODEL_H_

#include <QDialog>
#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include "qt_util.h"

struct HistParams
{
  bool visible {true};
  QColor color {Qt::black};

  int x1 {std::numeric_limits<int>::min()},
      x2{std::numeric_limits<int>::max()};
  int y1 {std::numeric_limits<int>::min()},
      y2{std::numeric_limits<int>::max()};

  int width() const { return x2 - x1; }
  int height() const { return y2 - y1; }
  int center_x() const { return ((x2 + x1) / 2); }
  int center_y() const { return ((y2 + y1) / 2); }

  void set_width(int w);
  void set_height(int h);
  void set_center_x(int x);
  void set_center_y(int y);

  double cutoff {0};
};

class BoxesModel : public QAbstractTableModel
{
  Q_OBJECT

private:
  QVector<HistParams> &parameters_;

signals:
  void data_changed();
  void editing_finished();

public:
  BoxesModel(QVector<HistParams> &, QObject *parent = 0);
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
