#ifndef SPECIAL_DELEGATE_H_
#define SPECIAL_DELEGATE_H_

#include <QStyledItemDelegate>
#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>

class SpecialDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  SpecialDelegate(QObject *parent = 0): QStyledItemDelegate(parent) {}
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const Q_DECL_OVERRIDE;

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;
  void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const Q_DECL_OVERRIDE;
  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const Q_DECL_OVERRIDE;

private slots:
  void intValueChanged(int);

signals:
  void begin_editing() const;
  void edit_integer(QModelIndex index, QVariant value, int role);

private:
  mutable QModelIndex edited_idx_;

};

#endif
