#include <QSettings>
#include <QRegExp>
#include "SearchList.h"
#include "CustomLogger.h"
#include <QGridLayout>
#include <QBoxLayout>
#include <QKeyEvent>
#include <QPushButton>

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
  if (subset_list_->selectedItems().isEmpty() && !filtered_set_.empty())
    subset_list_->item(0)->setSelected(true);
  subset_list_->setFocus();
}

void SearchList::setList(QStringList lst)
{
  filter_->setList(lst);
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

  subset_list_->clear();
  subset_list_->addItems(filtered_set_);

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

bool popupSearchDialog(QPushButton* button, QStringList choices)
{
  SearchDialog* popup = new SearchDialog();
  popup->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

  int maxwidth = button->rect().width();
  QFontMetrics fm(button->font());
  for (auto &choice : choices)
    maxwidth = std::max(maxwidth, fm.width(choice) + 30);
  popup->setList(choices);
  popup->Select(button->text());
  QRect rect;
  rect.setTopLeft(button->mapToGlobal(button->rect().topLeft()));
  rect.setWidth(maxwidth);
  rect.setHeight(std::min(250, (fm.height()+5) * choices.size()));
  popup->setGeometry(rect);
  popup->setFilterVisible(choices.size() >= 15);

  bool result = popup->exec();

  auto selection = popup->selection();
  delete popup;

  if (result)
    button->setText(selection);

  return result;
}

