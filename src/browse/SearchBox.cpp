#include <QSettings>
#include <QRegExp>
#include "SearchBox.h"
#include "CustomLogger.h"
#include <QGridLayout>
#include <QKeyEvent>

SearchBox::SearchBox(QWidget *parent)
  : QWidget(parent)
{
  filter_ = new QLineEdit;
  filter_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
  filter_->setMinimumSize(0, 24);
  connect(filter_, SIGNAL(textChanged(QString)), this, SLOT(filterChanged(QString)));

  QGridLayout* mainGrid = new QGridLayout;
  mainGrid->addWidget(filter_, 0, 0, -1, -1);
  mainGrid->setMargin(0);
  setLayout(mainGrid);
}

void SearchBox::setList(QStringList lst)
{
  selected_set_ = set_ = lst;
  filterChanged(filter_->text());
}

void SearchBox::setFilter(QString filter)
{
  filter_->setText(filter);
  filterChanged(filter);
}

QString SearchBox::filter() const
{
  return filter_->text();
}

QStringList SearchBox::selection() const
{
  return selected_set_;
}

void SearchBox::filterChanged(QString arg1)
{
  selected_set_ = clever_search(set_, arg1.split(" "));
  selected_set_.sort(Qt::CaseInsensitive);
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


void SearchBox::focusInEvent( QFocusEvent* e )
{
  QWidget::focusInEvent(e);
  filter_->setFocus();
}

void SearchBox::keyPressEvent(QKeyEvent *event)
{
  if ((event->key() == Qt::Key_Down)  ||
      (event->key() == Qt::Key_Enter) ||
      (event->key() == Qt::Key_Return))
    emit acceptedFilter();
  QWidget::keyPressEvent(event);
}



