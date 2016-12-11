#ifndef SEARCH_BOX_H
#define SEARCH_BOX_H

#include <QWidget>
#include <QItemSelection>
#include <QLineEdit>

class SearchBox : public QWidget
{
  Q_OBJECT

public:
  explicit SearchBox(QWidget *parent = 0);

  void setList(QStringList lst);
  QStringList selection() const;

  QString filter() const;

  void setFilter(QString filter);

signals:
  void selectionChanged();
  void acceptedFilter();

private slots:

  void filterChanged(QString);

protected:
  void focusInEvent( QFocusEvent* e ) Q_DECL_OVERRIDE;
  void keyPressEvent(QKeyEvent*) Q_DECL_OVERRIDE;

private:
  QLineEdit* filter_;

  QStringList set_;
  QStringList selected_set_;

  QStringList clever_search(const QStringList& list, QStringList queries);
  bool clever_test(const QString& string, const QString& query);
};

#endif
