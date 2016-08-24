#ifndef tpcc_H
#define tpcc_H

#include <QMainWindow>

#include "ViewEvent.h"
#include "Analyzer.h"

#include "ThreadClassify.h"


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
  std::shared_ptr<NMX::FileHDF5> reader_;

  NMX::Dimensions xdims_;
  NMX::Dimensions ydims_;

  ViewEvent   *event_viewer_;
  Analyzer    *analyzer_;

  ThreadClassify thread_classify_;

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

  void run_complete();

  void update_progress(double percent_done);

  void table_changed(double);
  std::map<std::string, double> collect_params();

  void on_pushStop_clicked();
  void on_pushStart_clicked();
  void on_comboGroup_activated(const QString &arg1);
  void on_pushNewGroup_clicked();

  void populate_combo();
};

#endif // tpcc_H
