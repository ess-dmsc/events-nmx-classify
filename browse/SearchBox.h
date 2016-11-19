#ifndef SEARCH_BOX_H
#define SEARCH_BOX_H

#include <QWidget>
#include <QItemSelection>

namespace Ui {
class SearchBox;
}


class SearchBox : public QWidget
{
  Q_OBJECT

public:
  explicit SearchBox(QWidget *parent = 0);
  ~SearchBox();

  void setList(QStringList lst);
  QStringList selection() const;

signals:
  void selectionChanged();

private slots:

  void on_lineFilter_textChanged(const QString &arg1);
  void listSelectionChanged();

private:
  Ui::SearchBox *ui;

  QStringList set_;
  QStringList selected_set_;

};

#endif
