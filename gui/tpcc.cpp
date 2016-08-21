#include <QSettings>
#include <utility>
#include <numeric>
#include <cstdint>

#include "tpcc.h"
#include "ui_tpcc.h"
#include "custom_timer.h"

#include "qt_util.h"

#include "ThreadAnalysis.h"

#include <QCloseEvent>

tpcc::tpcc(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::tpcc)
{
  qRegisterMetaType<std::shared_ptr<EntryList>>("std::shared_ptr<EntryList>");
  qRegisterMetaType<std::shared_ptr<MultiHists>>("std::shared_ptr<MultiHists>");

  CustomLogger::initLogger();
  ui->setupUi(this);

  event_viewer_ = new ViewEvent();
  ui->tabWidget->addTab(event_viewer_, "Event viewer");
  connect(this, SIGNAL(enableIO(bool)), event_viewer_, SLOT(enableIO(bool)));

  analyzer_ = new Analyzer();
  ui->tabWidget->addTab(analyzer_, "Analyzer");
  connect(analyzer_, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(this, SIGNAL(enableIO(bool)), analyzer_, SLOT(enableIO(bool)));

  connect(&thread_classify_, SIGNAL(data_ready(double)),
          this, SLOT(update_progress(double)));
  connect(&thread_classify_, SIGNAL(run_complete()), this, SLOT(run_complete()));

  ui->tableParams->verticalHeader()->hide();
  ui->tableParams->setColumnCount(2);
  ui->tableParams->setHorizontalHeaderLabels({"name", "value"});
  ui->tableParams->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableParams->setSelectionMode(QAbstractItemView::NoSelection);
  ui->tableParams->horizontalHeader()->setStretchLastSection(true);
  ui->tableParams->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  NMX::Event evt;
  auto valnames = evt.categories();
  ui->tableParams->setRowCount(valnames.size());
  int i = 0;
  for (auto &name : valnames)
  {
    QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(name));
    ui->tableParams->setItem(i, 0, item);

    QDoubleSpinBox *spinBox = new QDoubleSpinBox(this);
    spinBox->setMinimum(0);
    spinBox->setMaximum(std::numeric_limits<short>::max());
    spinBox->setValue( evt.get_value(name) );
    connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(table_changed(double)));
    ui->tableParams->setCellWidget( i, 1, spinBox );

    i++;
  }

  thread_classify_.set_refresh_frequency(2);

  loadSettings();
}

tpcc::~tpcc()
{
  CustomLogger::closeLogger();
  delete ui;
}

void tpcc::table_changed(double y)
{
  auto params = collect_params();
  event_viewer_->set_params(params);
  analyzer_->set_params(params);
}

std::map<std::string, double> tpcc::collect_params()
{
  std::map<std::string, double> ret;
  for (int i=0; i < ui->tableParams->rowCount(); ++i)
  {
    std::string name = ui->tableParams->model()->data(ui->tableParams->model()->index(i,0)).toString().toStdString();
    double val = static_cast<QDoubleSpinBox*>(ui->tableParams->cellWidget(i, 1))->value();
    ret[name] = val;
  }
  return ret;
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

  if (fileName.isEmpty())
    return;
  if (open_file(fileName));
    event_viewer_->set_params(collect_params());
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
  QString fileName = QFileDialog::getOpenFileName(this, "Load TPC data", data_directory_, "HDF5 (*.h5)");
  if (!validateFile(this, fileName, false))
    return;

  open_file(fileName);
}

bool tpcc::open_file(QString fileName)
{
  data_directory_ = path_of_file(fileName);

  reader_ = std::make_shared<NMX::FileHDF5>(fileName.toStdString());

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
  ui->tableParams->setEnabled(enable);

  bool en = reader_ && reader_->event_count() && enable;
  ui->pushStart->setEnabled(en);

  emit enableIO(enable);
}

void tpcc::on_pushStop_clicked()
{
  thread_classify_.terminate();
}

void tpcc::on_pushStart_clicked()
{
//  clear();

  if (!reader_|| !reader_->event_count())
    return;

  ui->pushStop->setEnabled(true);
  ui->progressBar->setValue(0);

  toggleIO(false);
  thread_classify_.go(reader_, collect_params());
}

void tpcc::update_progress(double percent_done)
{
  ui->progressBar->setValue(percent_done);
}


void tpcc::run_complete()
{
  toggleIO(true);
  ui->pushStop->setEnabled(false);
}
