#include <QSettings>
#include <utility>
#include <numeric>
#include <cstdint>
#include "tpcReaderIndexedBin.h"
#include "tpcMimicVMMx.h"
#include "tpcFindEntry.h"

#include "tpcc.h"
#include "ui_tpcc.h"
#include "custom_timer.h"

#include "qt_util.h"

#include <QCloseEvent>

tpcc::tpcc(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::tpcc)
{
  qRegisterMetaType<std::shared_ptr<EntryList>>("std::shared_ptr<EntryList>");

  CustomLogger::initLogger();
  ui->setupUi(this);

  event_viewer_ = new EventViewer();
  ui->tabWidget->addTab(event_viewer_, "Event viewer");
  connect(this, SIGNAL(enableIO(bool)), event_viewer_, SLOT(enableIO(bool)));

  analyzer_ = new Analyzer();
  ui->tabWidget->addTab(analyzer_, "Analyzer");
  connect(analyzer_, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(this, SIGNAL(enableIO(bool)), analyzer_, SLOT(enableIO(bool)));

  loadSettings();
}

tpcc::~tpcc()
{
  CustomLogger::closeLogger();
  delete ui;
}

void tpcc::closeEvent(QCloseEvent *event)
{
  saveSettings();
  event->accept();
}

void tpcc::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  QRect myrect = settings.value("position",QRect(20,20,1234,650)).toRect();
  setGeometry(myrect);
  data_directory_ = settings.value("data_directory", "").toString();

  QString fileName = settings.value("recent_file").toString();

  if (!fileName.isEmpty())
    open_file(fileName);
}

void tpcc::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("position", this->geometry());
  settings.setValue("data_directory", data_directory_);
}

void tpcc::on_toolOpen_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Load TPC data", data_directory_, "indexed binary (*.bin)");
  if (!validateFile(this, fileName, false))
    return;

  open_file(fileName);
}

bool tpcc::open_file(QString fileName)
{
  data_directory_ = path_of_file(fileName);

  reader_ = std::make_shared<TPC::ReaderIndexedBin>(fileName.toStdString());

  event_viewer_->set_new_source(reader_, xdims_, ydims_);
  analyzer_->set_new_source(reader_, xdims_, ydims_);

  int evt_count = reader_->event_count();

  ui->labelOfTotal->setText(fileName);

  QSettings settings;
  settings.beginGroup("Program");
  if (evt_count)
    settings.setValue("recent_file", fileName);
  else
    settings.setValue("recent_file", QVariant());

  return (evt_count > 0);
}

void tpcc::toggleIO(bool enable)
{
  ui->toolOpen->setEnabled(enable);
  emit enableIO(enable);
}
