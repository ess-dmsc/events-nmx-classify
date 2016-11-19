#include <QSettings>
#include <QRegExp>
#include "SearchBox.h"
#include "ui_SearchBox.h"

SearchBox::SearchBox(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::SearchBox)
{
  ui->setupUi(this);
//  ui->listSubset->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->listSubset->setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(ui->listSubset, SIGNAL(itemSelectionChanged()), this, SLOT(listSelectionChanged()));
}

SearchBox::~SearchBox()
{
  delete ui;
}

void SearchBox::setList(QStringList lst)
{
  set_ = lst;
  on_lineFilter_textChanged(ui->lineFilter->text());
}

QStringList SearchBox::selection() const
{
  if (ui->listSubset->selectedItems().empty())
    return selected_set_;

  QStringList ret;
  for (auto i : ui->listSubset->selectedItems())
    ret.push_back(i->text());
  return ret;
}

void SearchBox::listSelectionChanged()
{
  emit selectionChanged();
}

void SearchBox::on_lineFilter_textChanged(const QString &arg1)
{
  selected_set_ = set_;
  QStringList filters = arg1.split(" ");
  for (QString f : filters)
  {
    if (f.isEmpty())
      continue;

    if (f.at(0) == QChar('~'))
    {
      QStringList s;
      f = f.mid(1);
      for (QString i : selected_set_)
        if (!i.contains(f))
          s.push_back(i);
      selected_set_ = s;
    }
    else if (f.at(f.size()-1) == QChar('!'))
    {
      QStringList s;
      f = f.mid(0, f.size()-1);
      for (QString i : selected_set_)
        if (i.endsWith(f))
          s.push_back(i);
      selected_set_ = s;
    }
    else
    {
      QRegExp regExp(f, Qt::CaseInsensitive, QRegExp::Wildcard);
      selected_set_ = selected_set_.filter(regExp);
    }
  }

  ui->listSubset->clear();
  ui->listSubset->addItems(selected_set_);

  emit selectionChanged();
}
