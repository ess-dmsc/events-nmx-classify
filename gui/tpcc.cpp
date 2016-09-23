#include <QSettings>

#include "tpcc.h"
#include "ui_tpcc.h"

#include "qt_util.h"
#include "CustomLogger.h"
#include "Variant.h"


tpcc::tpcc(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::tpcc)
{
  qRegisterMetaType<std::shared_ptr<EntryList>>("std::shared_ptr<EntryList>");

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

  connect(analyzer_, SIGNAL(select_indices(std::set<size_t>)), event_viewer_, SLOT(set_indices(std::set<size_t>)));

  ui->tableParams->verticalHeader()->hide();
  ui->tableParams->setColumnCount(2);
  ui->tableParams->setHorizontalHeaderLabels({"name", "value"});
  ui->tableParams->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableParams->setSelectionMode(QAbstractItemView::NoSelection);
  ui->tableParams->horizontalHeader()->setStretchLastSection(true);
  ui->tableParams->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  parameters_ = NMX::Event().parameters();
  display_params();

  thread_classify_.set_refresh_frequency(2);
  ui->comboGroup->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  QTimer::singleShot(1000, this, SLOT(loadSettings()));
}

tpcc::~tpcc()
{
  CustomLogger::closeLogger();
  delete ui;
}

void tpcc::table_changed(double /*v*/)
{
  collect_params();
  event_viewer_->set_params(parameters_);
}

void tpcc::collect_params()
{
  for (int i=0; i < ui->tableParams->rowCount(); ++i)
  {
    std::string name = ui->tableParams->model()->data(ui->tableParams->model()->index(i,0)).toString().toStdString();
    double val = static_cast<QDoubleSpinBox*>(ui->tableParams->cellWidget(i, 1))->value();
    parameters_[name].value = Variant::from_float(val);
  }
}

void tpcc::display_params()
{
  ui->tableParams->setRowCount(parameters_.size());
  int i = 0;
  for (auto &param : parameters_)
  {
    QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(param.first));
    ui->tableParams->setItem(i, 0, item);

    QDoubleSpinBox *spinBox = new QDoubleSpinBox(this);
    spinBox->setMinimum(0);
    spinBox->setMaximum(std::numeric_limits<short>::max());
    spinBox->setValue( param.second.value.as_float() );
    connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(table_changed(double)));
    ui->tableParams->setCellWidget( i, 1, spinBox );

    i++;
  }
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

  open_file(fileName);
}

void tpcc::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("position", this->geometry());
  settings.setValue("data_directory", data_directory_);
}

bool tpcc::open_file(QString fileName)
{
  data_directory_ = path_of_file(fileName);

  reader_ = std::make_shared<NMX::FileHDF5>(fileName.toStdString());

  event_viewer_->set_new_source(reader_);

  int evt_count = reader_->event_count();

  QSettings settings;
  settings.beginGroup("Program");
  if (evt_count)
  {
    ui->pushOpen->setText("  " + fileName);
    settings.setValue("recent_file", fileName);
    event_viewer_->set_params(parameters_);
  }
  else
  {
    ui->pushOpen->setText("");
    settings.setValue("recent_file", QVariant());
  }

  populate_combo();

  if (!reader_->analysis_groups().empty())
    ui->comboGroup->setCurrentText(QString::fromStdString(reader_->analysis_groups().front()));
  on_comboGroup_activated("");

  return (evt_count > 0);
}

void tpcc::toggleIO(bool enable)
{
  auto names = reader_->analysis_groups();

  ui->pushOpen->setEnabled(enable);
  ui->tableParams->setEnabled(enable && reader_ && !reader_->num_analyzed());

  ui->comboGroup->setEnabled(enable && (names.size() > 0));

  ui->pushNewGroup->setEnabled(enable && reader_ && reader_->event_count());
  ui->pushDeleteGroup->setEnabled(enable && reader_ && names.size());

  bool en = reader_ && reader_->event_count()
      &&  (reader_->num_analyzed() < reader_->event_count()) && (names.size() > 0) && enable;
  ui->pushStart->setEnabled(en);

  emit enableIO(enable);
}

void tpcc::on_pushStop_clicked()
{
  thread_classify_.terminate();
}

void tpcc::on_pushStart_clicked()
{
  if (!reader_|| !reader_->event_count())
    return;

  ui->pushStop->setEnabled(true);

  toggleIO(false);
  collect_params();

  DBG << "Analyzing using params:";

  for (auto &param : parameters_)
    DBG << param.first << " (" << param.second.value.type_name() << ") = " << param.second.value.to_string();

  thread_classify_.go(reader_, parameters_);
}

void tpcc::update_progress(double percent_done)
{
  ui->progressBar->setValue(percent_done);
}


void tpcc::run_complete()
{
  toggleIO(true);
  ui->pushStop->setEnabled(false);

  reader_->save_analysis();
  analyzer_->set_new_source(reader_);
}

void tpcc::on_comboGroup_activated(const QString& /*arg1*/)
{
  std::string name = ui->comboGroup->currentText().toStdString();
  reader_->load_analysis(name);

  double percent = double(reader_->num_analyzed()+1) / double(reader_->event_count()) * 100;

  parameters_ = reader_->get_parameters();
  if (parameters_.empty())
    parameters_ = NMX::Event().parameters();

  display_params();

  ui->progressBar->setValue(percent);

  analyzer_->set_new_source(reader_);

  toggleIO(true);
}

void tpcc::on_pushNewGroup_clicked()
{
  bool ok = false;
  QString text = QInputDialog::getText(this, "New analysis group",
                                           "Group name:", QLineEdit::Normal,
                                           "", &ok);
  if (!ok)
    return;

  if (text.isEmpty())
    return;

  //must not have slashes!

  //must not already exist
  for (auto &name : reader_->analysis_groups())
    if (name == text.toStdString())
      return;

  reader_->create_analysis(text.toStdString());
  populate_combo();

  ui->comboGroup->setCurrentText(text);
  on_comboGroup_activated(text);
}

void tpcc::populate_combo()
{
  ui->comboGroup->clear();
  for (auto &name : reader_->analysis_groups())
    ui->comboGroup->addItem(QString::fromStdString(name));
  toggleIO(true);
}

void tpcc::on_pushDeleteGroup_clicked()
{
  reader_->delete_analysis(ui->comboGroup->currentText().toStdString());
  populate_combo();
  on_comboGroup_activated("");
}

void tpcc::on_pushOpen_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Load TPC data", data_directory_, "HDF5 (*.h5)");
  if (!validateFile(this, fileName, false))
    return;

  open_file(fileName);
}
