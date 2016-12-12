#ifndef Browser_H
#define Browser_H

#include <QMainWindow>

#include "ViewEvent.h"
#include "Analyzer.h"
#include "AggregateReview.h"

namespace Ui {
class Browser;
}

class Browser : public QMainWindow
{
  Q_OBJECT

public:
  explicit Browser(QWidget *parent = 0);
  ~Browser();

signals:
  void enableIO(bool);

protected:
  void closeEvent(QCloseEvent*);

private slots:
  void saveSettings();
  void loadSettings();

  void toggleIO(bool);

  void on_comboGroup_activated(const QString &arg1);
  void on_pushNewGroup_clicked();
  void on_pushDeleteGroup_clicked();
  void populate_combo();
  void on_pushOpen_clicked();

  void digDownTown(QString dset, QString metric, QString file);

  void on_pushMetricsGlossary_clicked();

private:
  Ui::Browser *ui;

  QString data_directory_;
  std::shared_ptr<NMX::FileAPV> reader_;

  ViewEvent   *event_viewer_;
  Analyzer    *analyzer_;
  AggregateReview *review_;

  void open_file(QString fileName);

};

#endif // Browser_H
