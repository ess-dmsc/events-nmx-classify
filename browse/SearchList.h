#ifndef SEARCH_LIST_H
#define SEARCH_LIST_H

#include <QWidget>
#include <QItemSelection>

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
  QStringList selection() const;

  QString filter() const;
  void setFilter(QString filter);

signals:
  void selectionChanged();

private slots:
  void filterSelectionChanged();
  void listSelectionChanged();

private:
  Ui::SearchList *ui;

  QStringList selected_set_;
};

#endif
