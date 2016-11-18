#include "SpecialDelegate.h"
#include "qt_util.h"
#include <QComboBox>
#include <QPainter>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>
#include <QColorDialog>
#include <QComboBox>

#include <QApplication>
#include "CustomLogger.h"

void SpecialDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
  if (index.data().type() == QVariant::Color)
  {
    QColor thisColor = qvariant_cast<QColor>(index.data());
    painter->fillRect(option.rect, thisColor);
    return;
  }
  else if (index.data().type() == QVariant::LongLong)
  {
    QString text = QString::number(index.data().toInt());
    int flags = Qt::TextWordWrap | Qt::AlignVCenter;

    painter->save();
    if (option.state & QStyle::State_Selected)
    {
      painter->fillRect(option.rect, option.palette.highlight());
      painter->setPen(option.palette.highlightedText().color());
    }
    else
      painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));

    painter->drawText(option.rect, flags, text);
    painter->restore();
  }
  else
    QStyledItemDelegate::paint(painter, option, index);
}

QSize SpecialDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
  if (index.data().type() == QVariant::Color)
  {
    QColor thisColor = qvariant_cast<QColor>(index.data());
    QString text = thisColor.name();
    QRect r = option.rect;
    QFontMetrics fm(QApplication::font());
    QRect qr = fm.boundingRect(r, Qt::AlignLeft | Qt::AlignVCenter, text);
    QSize size(qr.size());
    return size;
  }
  else if (index.data().type() == QVariant::LongLong)
  {
    QString text = QString::number(index.data().toInt());
    QRect r = option.rect;
    QFontMetrics fm(QApplication::font());
    QRect qr = fm.boundingRect(r, Qt::AlignLeft | Qt::AlignVCenter, text);
    QSize size(qr.size());
    return size;
  }
  else
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *SpecialDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const

{
  emit begin_editing();

  if (index.data(Qt::EditRole).canConvert<Variant>())
  {
    Variant itemData = qvariant_cast<Variant>(index.data(Qt::EditRole));
    if (itemData.type() == Variant::code::type_bool)
    {
      QComboBox *editor = new QComboBox(parent);
      editor->addItem("True", QVariant::fromValue(true));
      editor->addItem("False", QVariant::fromValue(false));
      return editor;
    }
    else if (itemData.type() == Variant::code::type_int)
    {
      QSpinBox *editor = new QSpinBox(parent);
      editor->setMinimum(std::numeric_limits<int>::min());
      editor->setMaximum(std::numeric_limits<int>::max());
      editor->setSingleStep(1);
      edited_idx_ = index;
      connect(editor, SIGNAL(valueChanged(int)), this, SLOT(intValueChanged(int)));
      return editor;
    }
    else if (itemData.type() == Variant::code::type_uint)
    {
      QSpinBox *editor = new QSpinBox(parent);
      editor->setMinimum(0);
      editor->setMaximum(std::numeric_limits<int>::max());
      editor->setSingleStep(1);
      edited_idx_ = index;
      connect(editor, SIGNAL(valueChanged(int)), this, SLOT(intValueChanged(int)));
      return editor;
    }
    else if (itemData.type() == Variant::code::type_float)
    {
      QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
      editor->setMinimum(std::numeric_limits<double>::min());
      editor->setMaximum(std::numeric_limits<double>::max());
      editor->setSingleStep(1);
      edited_idx_ = index;
//      connect(editor, SIGNAL(valueChanged(int)), this, SLOT(intValueChanged(int)));
      return editor;
    }
  }

  if (index.data().type() == QVariant::Bool)
  {
    QComboBox *editor = new QComboBox(parent);
    editor->addItem("True", QVariant::fromValue(true));
    editor->addItem("False", QVariant::fromValue(false));
    return editor;
  }
  else if (index.data().canConvert(QVariant::LongLong))
  {
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(std::numeric_limits<int>::min());
    editor->setMaximum(std::numeric_limits<int>::max());
    editor->setSingleStep(1);

    edited_idx_ = index;

    connect(editor, SIGNAL(valueChanged(int)), this, SLOT(intValueChanged(int)));

    return editor;
  }
  else
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void SpecialDelegate::setEditorData ( QWidget *editor, const QModelIndex &index ) const
{

  if (index.data(Qt::EditRole).canConvert<Variant>())
  {
    Variant itemData = qvariant_cast<Variant>(index.data(Qt::EditRole));

    if (itemData.type() == Variant::code::type_bool)
    {
      if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
      {
        int cbIndex = cb->findData(itemData.as_bool().val());
        if(cbIndex >= 0)
          cb->setCurrentIndex(cbIndex);
      }
    }
    else if (itemData.type() == Variant::code::type_int)
    {
      if (QSpinBox *sb = qobject_cast<QSpinBox *>(editor))
        sb->setValue(itemData.as_int().val());
    }
    else if (itemData.type() == Variant::code::type_uint)
    {
      if (QSpinBox *sb = qobject_cast<QSpinBox *>(editor))
        sb->setValue(itemData.as_uint().val());
    }
    else if (itemData.type() == Variant::code::type_float)
    {
      if (QDoubleSpinBox *sb = qobject_cast<QDoubleSpinBox *>(editor))
        sb->setValue(itemData.as_float().val());
    }
  }
  else if (QSpinBox *sb = qobject_cast<QSpinBox *>(editor))
      sb->setValue(index.data().toLongLong());
  else if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
  {
    int cbIndex = cb->findData(QVariant::fromValue(index.data()));
    if(cbIndex >= 0)
      cb->setCurrentIndex(cbIndex);
  }
  else
    QStyledItemDelegate::setEditorData(editor, index);
}

void SpecialDelegate::setModelData ( QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index ) const
{
  if (QSpinBox *sb = qobject_cast<QSpinBox *>(editor))
    model->setData(index, QVariant::fromValue(sb->value()), Qt::EditRole);
  else if (QDoubleSpinBox *sb = qobject_cast<QDoubleSpinBox *>(editor))
    model->setData(index, QVariant::fromValue(sb->value()), Qt::EditRole);
  else if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
    model->setData(index, QVariant::fromValue(cb->currentText() == "True"), Qt::EditRole);
  else
    QStyledItemDelegate::setModelData(editor, model, index);
}

void SpecialDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/) const
{
  editor->setGeometry(option.rect);
}

void SpecialDelegate::intValueChanged(int val)
{
  long long v = val;
  emit edit_integer(edited_idx_, QVariant::fromValue(v), Qt::EditRole);
}
