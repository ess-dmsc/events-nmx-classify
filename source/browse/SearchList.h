#ifndef SEARCH_LIST_H
#define SEARCH_LIST_H

#include <QWidget>
#include <QItemSelection>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include "SearchBox.h"

class MyListWidget : public QListWidget
{
  Q_OBJECT
protected:
  void keyPressEvent(QKeyEvent*) Q_DECL_OVERRIDE;
signals:
  void pressedEnter();
};

class SearchList : public QWidget
{
  Q_OBJECT

public:
  explicit SearchList(QWidget *parent = 0);

  void setList(QStringList lst);
  void setSingleSelection(bool);
  QStringList selection() const;
  void Select(QString sel);

  QString filter() const;
  void setFilter(QString filter);
  void setFilterLabel(QString txt);
  void setFilterVisible(bool);

signals:
  void selectionChanged();
  void doubleClickedOne(QString);

private slots:
  void filterChanged();
  void listSelectionChanged();
  void double_click();
  void filterAccepted();

protected:
  void focusInEvent( QFocusEvent* e ) Q_DECL_OVERRIDE;

private:
  QLabel* filter_label_;
  SearchBox* filter_;
  MyListWidget* subset_list_;

  QStringList set_;
  QStringList filtered_set_;
  QStringList selected_set_;
};


class SearchDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SearchDialog(QWidget *parent = 0);

  void setList(QStringList lst) { widget_->setList(lst); }
  QString selection() const { return selection_; }
  void Select(QString sel) { widget_->Select(sel); }

  QString filter() const { return widget_->filter(); }
  void setFilter(QString filter) { widget_->setFilter(filter); }
  void setFilterLabel(QString txt) { widget_->setFilterLabel(txt); }
  void setFilterVisible(bool vis) { widget_->setFilterVisible(vis); }

protected:
  void focusInEvent(QFocusEvent* e) Q_DECL_OVERRIDE;

signals:
  void selectionChanged();
  void doubleClickedOne(QString);

private slots:
  void doubleClicked(QString sel);

private:
  SearchList* widget_;
  QString selection_;
};

bool popupSearchDialog(QPushButton *button,
                       QStringList choices,
                       QString default_filter = "",
                       QString filter_label = "Filter:");


class FilterCombo : public QPushButton
{
  Q_OBJECT

public:
  explicit FilterCombo(QWidget *parent = 0);

  void setList(QStringList lst) { list_ = lst; }
  void setDefaultFilter(QString filter) { default_filter_ = filter; }
  void setFilterLabel(QString txt) { filter_label_ = txt; }

signals:
  void selectionChanged(QString);

private slots:
  void clickedButton();

private:
  QString default_filter_;
  QString filter_label_ {"Filter:"};
  QStringList list_;
};


#endif
