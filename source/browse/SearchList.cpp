#include <QSettings>
#include <QRegExp>
#include "SearchList.h"
#include "CustomLogger.h"
#include <QGridLayout>
#include <QBoxLayout>
#include <QKeyEvent>

void MyListWidget::keyPressEvent(QKeyEvent *event)
{
  if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return))
    emit pressedEnter();
  QListWidget::keyPressEvent(event);
}

SearchList::SearchList(QWidget *parent)
  : QWidget(parent)
{
  filter_label_ = new QLabel;
  filter_label_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  filter_label_->setMinimumSize(30, 24);
  filter_label_->setText("Filter:");

  filter_ = new SearchBox;
  filter_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
  filter_->setMinimumSize(0, 24);
  connect(filter_, SIGNAL(selectionChanged()), this, SLOT(filterChanged()));
  connect(filter_, SIGNAL(acceptedFilter()), this, SLOT(filterAccepted()));

  subset_list_ = new MyListWidget;
  subset_list_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  subset_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(subset_list_, SIGNAL(pressedEnter()), this, SLOT(double_click()));
  connect(subset_list_, SIGNAL(itemSelectionChanged()), this, SLOT(listSelectionChanged()));
  connect(subset_list_, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
          this, SLOT(double_click()));

  QHBoxLayout* filter_layout = new QHBoxLayout;
  filter_layout->addWidget(filter_label_);
  filter_layout->addWidget(filter_);

  QGridLayout* mainGrid = new QGridLayout;
  mainGrid->addLayout(filter_layout, 0, 0, 1, -1);
  mainGrid->addWidget(subset_list_, 1, 0, 1, -1);
  mainGrid->setMargin(0);
  setLayout(mainGrid);
}

void SearchList::setSingleSelection(bool ss)
{
  if (ss)
    subset_list_->setSelectionMode(QAbstractItemView::SingleSelection);
  else
    subset_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
}


void SearchList::setFilterLabel(QString txt)
{
  filter_label_->setText(txt);
  filter_label_->setVisible(filter_->isVisible() && !txt.isEmpty());
}

void SearchList::setFilterVisible(bool vis)
{
  filter_->setVisible(vis);
  filter_label_->setVisible(vis && !filter_label_->text().isEmpty());

  if (vis)
    filter_->setFocus();
  else
    subset_list_->setFocus();
}

void SearchList::filterAccepted()
{
  subset_list_->setFocus();
}

void SearchList::setList(QStringList lst)
{
  filter_->setList(lst);
  filterChanged();
}

void SearchList::setDescriptions(QMap<QString,QString> descriptions)
{
  descriptions_ = descriptions;
  filterChanged();
}

void SearchList::setFilter(QString filter)
{
  filter_->setFilter(filter);
}

QString SearchList::filter() const
{
  return filter_->filter();
}

QStringList SearchList::selection() const
{
  return selected_set_;
}

void SearchList::double_click()
{
  for (auto i : subset_list_->selectedItems())
  {
    emit doubleClickedOne(i->text());
    return;
  }
  emit doubleClickedOne("");
}

void SearchList::filterChanged()
{
  filtered_set_ = filter_->selection();

  auto set = filtered_set_;
  if (descriptions_.size())
  {
    set.clear();
    for (auto i : filtered_set_)
    {
      if (descriptions_.count(i))
        i = "<b>" + i +  "</b>   " + descriptions_.value(i);
      set.push_back(i);
    }
  }

  subset_list_->clear();
  subset_list_->addItems(set);

  selected_set_ = filtered_set_;

  emit selectionChanged();
}

void SearchList::listSelectionChanged()
{
  if (subset_list_->selectedItems().isEmpty())
    selected_set_ = filtered_set_;
  else
  {
    selected_set_.clear();
    for (auto i : subset_list_->selectedItems())
      selected_set_.push_back(i->text());
  }

  emit selectionChanged();
}

void SearchList::Select(QString sel)
{
  if (descriptions_.count(sel))
    sel += "   " + descriptions_.value(sel);
  for (int i=0; i < subset_list_->count(); i++)
  {
    if (subset_list_->item(i)->text() == sel)
    {
      subset_list_->clearSelection();
      subset_list_->item(i)->setSelected(true);
      subset_list_->setCurrentRow(i);
      return;
    }
  }
  emit selectionChanged();
}

void SearchList::focusInEvent( QFocusEvent* e )
{
  QWidget::focusInEvent(e);
  if (filter_->isVisible())
    filter_->setFocus();
  else
    subset_list_->setFocus();
}





SearchDialog::SearchDialog(QWidget *parent)
  : QDialog(parent)
{
  widget_ = new SearchList(this);
  widget_->setSingleSelection(true);

  connect(widget_, SIGNAL(doubleClickedOne(QString)), this, SLOT(doubleClicked(QString)));

  QGridLayout* mainGrid = new QGridLayout;
  mainGrid->setMargin(1);
  mainGrid->addWidget(widget_);
  setLayout(mainGrid);

  widget_->setFocus();
}

void SearchDialog::doubleClicked(QString sel)
{
  selection_ = sel;
  accept();
}

void SearchDialog::focusInEvent(QFocusEvent* e)
{
  QDialog::focusInEvent(e);
  widget_->setFocus();
}

