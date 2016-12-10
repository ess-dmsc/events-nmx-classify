#ifndef SEARCH_LIST_H
#define SEARCH_LIST_H

#include <QWidget>
#include <QItemSelection>
#include <QDialog>

namespace Ui {
class SearchList;
}

class SearchList : public QWidget
{
  Q_OBJECT

public:
  explicit SearchList(QWidget *parent = 0);
  ~SearchList();

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
  void filterSelectionChanged();
  void listSelectionChanged();
  void double_click();

private:
  Ui::SearchList *ui;

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
