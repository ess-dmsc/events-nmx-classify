#include <QSettings>
#include <QRegExp>
#include "SearchList.h"
#include "ui_SearchList.h"
#include "CustomLogger.h"

SearchList::SearchList(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::SearchList)
{
  ui->setupUi(this);
  ui->listSubset->setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(ui->listSubset, SIGNAL(itemSelectionChanged()), this, SLOT(listSelectionChanged()));
  connect(ui->searchBox, SIGNAL(selectionChanged()), this, SLOT(filterSelectionChanged()));
}

SearchList::~SearchList()
{
  delete ui;
}

void SearchList::setList(QStringList lst)
{
  ui->searchBox->setList(lst);
}

void SearchList::setFilter(QString filter)
{
  ui->searchBox->setFilter(filter);
}

QString SearchList::filter() const
{
  return ui->searchBox->filter();
}

QStringList SearchList::selection() const
{
  return selected_set_;
}

void SearchList::filterSelectionChanged()
{
  selected_set_ = ui->searchBox->selection();
  ui->listSubset->clear();
  ui->listSubset->addItems(selected_set_);

  emit selectionChanged();
}

void SearchList::listSelectionChanged()
{
  if (ui->listSubset->selectedItems().isEmpty())
    selected_set_ = ui->searchBox->selection();
  else
  {
    selected_set_.clear();
    for (auto i : ui->listSubset->selectedItems())
      selected_set_.push_back(i->text());
  }

  emit selectionChanged();
}

void SearchList::Select(QString sel)
{
  for (int i=0; i < ui->listSubset->count(); i++)
  {
    if (ui->listSubset->item(i)->text() == sel)
    {
      ui->listSubset->clearSelection();
      ui->listSubset->item(i)->setSelected(true);
      ui->listSubset->setCurrentRow(i);
      return;
    }
  }
  emit selectionChanged();
}

