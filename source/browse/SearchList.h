#ifndef SEARCH_LIST_H
#define SEARCH_LIST_H

#include <QWidget>
#include <QItemSelection>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
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
  void setDescriptions(QMap<QString,QString> descriptions);
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
  QMap<QString,QString> descriptions_;
};


class SearchDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SearchDialog(QWidget *parent = 0);

  void setList(QStringList lst) { widget_->setList(lst); }
  void setDescriptions(QMap<QString,QString> descriptions)
    { widget_->setDescriptions(descriptions);}
  QString selection() const { return selection_; }
  void Select(QString sel) { widget_->Select(sel); }

  QString filter() const { return widget_->filter(); }
  void setFilter(QString filter) { widget_->setFilter(filter); }
  void setFilterLabel(QString txt) { widget_->setFilterLabel(txt); }
  void setFilterVisible(bool vis) { widget_->setFilterVisible(vis); }

protected:
  void focusInEvent( QFocusEvent* e ) Q_DECL_OVERRIDE;

signals:
  void selectionChanged();
  void doubleClickedOne(QString);

private slots:
  void doubleClicked(QString sel);

private:
  SearchList* widget_;
  QString selection_;
};


#endif
