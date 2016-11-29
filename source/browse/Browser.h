#ifndef Browser_H
#define Browser_H

#include <QMainWindow>

#include "ViewEvent.h"
#include "Analyzer.h"
#include "AggregateReview.h"

#include "ThreadClassify.h"
#include "TableSettings.h"
#include "SpecialDelegate.h"


namespace Ui {
class Browser;
}

class Browser : public QMainWindow
{
  Q_OBJECT

public:
  explicit Browser(QWidget *parent = 0);
  ~Browser();

private:
  Ui::Browser *ui;

  QString data_directory_;
  std::shared_ptr<NMX::FileAPV> reader_;

  ViewEvent   *event_viewer_;
  Analyzer    *analyzer_;
  AggregateReview *review_;

  ThreadClassify thread_classify_;

  TableSettings settings_model_;
  SpecialDelegate params_delegate_;


  bool open_file(QString fileName);

signals:
  void enableIO(bool);

protected:
  void closeEvent(QCloseEvent*);

private slots:
  void saveSettings();
  void loadSettings();

  void toggleIO(bool);

  void run_complete();

  void update_progress(double percent_done);

  void table_changed();
  void display_params();

  void on_pushStop_clicked();
  void on_pushStart_clicked();

  void on_comboGroup_activated(const QString &arg1);
  void on_pushNewGroup_clicked();
  void on_pushDeleteGroup_clicked();
  void populate_combo();
  void on_pushOpen_clicked();
  void on_pushShowParams_clicked();

  void digDownTown(QString dset, QString metric, QString file);
};

#endif // Browser_H
