#ifndef tpcc_H
#define tpcc_H

#include <QMainWindow>

#include "CustomLogger.h"
#include "Reader.h"
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
  std::shared_ptr<NMX::Reader> reader_;

  NMX::Dimensions xdims_;
  NMX::Dimensions ydims_;

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
