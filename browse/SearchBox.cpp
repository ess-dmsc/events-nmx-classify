#include <QSettings>
#include <QRegExp>
#include "SearchBox.h"
#include "ui_SearchBox.h"
#include "CustomLogger.h"

SearchBox::SearchBox(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::SearchBox)
{
  ui->setupUi(this);
  ui->listSubset->setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(ui->listSubset, SIGNAL(itemSelectionChanged()), this, SLOT(listSelectionChanged()));
}

SearchBox::~SearchBox()
{
  delete ui;
}

void SearchBox::setList(QStringList lst)
{
  selected_set_ = set_ = lst;
  on_lineFilter_textChanged(ui->lineFilter->text());
}

void SearchBox::setFilter(QString filter)
{
  ui->lineFilter->setText(filter);
  on_lineFilter_textChanged(filter);
}

QString SearchBox::filter() const
{
  return ui->lineFilter->text();
}

QStringList SearchBox::selection() const
{
  if (ui->listSubset->selectedItems().isEmpty())
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
  selected_set_ = clever_search(set_, arg1.split(" "));
  selected_set_.sort(Qt::CaseInsensitive);
  ui->listSubset->clear();
  ui->listSubset->addItems(selected_set_);

  emit selectionChanged();
}

bool SearchBox::clever_test(const QString& string, const QString& query)
{
  if (query.isEmpty())
    return true;

  if (query.startsWith("~"))
    return !clever_test(string, query.mid(1));

  if (query.startsWith("!"))
    return string.startsWith(query.mid(1));

  if (query.endsWith("!"))
    return string.endsWith(query.mid(0,query.size()-1));

  return string.contains(query, Qt::CaseInsensitive);
}

QStringList SearchBox::clever_search(const QStringList& list, QStringList queries)
{
  if (queries.isEmpty())
    return list;
  QStringList filtered;
  QString filter = queries.front();
  queries.pop_front();
  for (QString string : list)
    if (clever_test(string, filter))
      filtered.push_back(string);
  return clever_search(filtered, queries);
}

