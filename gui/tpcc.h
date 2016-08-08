#ifndef tpcc_H
#define tpcc_H

#include <QMainWindow>

#include "custom_logger.h"
#include "tpcReader.h"
#include "widget_plot2d.h"

#include "event_viewer.h"
#include "analyzer.h"


namespace Ui {
class tpcc;
}

class tpcc : public QMainWindow
{
  Q_OBJECT

public:
  explicit tpcc(QWidget *parent = 0);
  ~tpcc();

private:
  Ui::tpcc *ui;

  QString data_directory_;
  std::shared_ptr<TPC::Reader> reader_;

  TPC::Dimensions xdims_;
  TPC::Dimensions ydims_;

  EventViewer *event_viewer_;
  Analyzer    *analyzer_;

  //helper functions
  void saveSettings();
  void loadSettings();

  bool open_file(QString fileName);

signals:
  void enableIO(bool);

protected:
  void closeEvent(QCloseEvent*);

private slots:
  void on_toolOpen_clicked();
  void toggleIO(bool);

};

#endif // tpcc_H
