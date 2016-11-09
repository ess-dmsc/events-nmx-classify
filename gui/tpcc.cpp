#include <QSettings>

#include "tpcc.h"
#include "ui_tpcc.h"

#include "CustomLogger.h"
#include "Variant.h"

tpcc::tpcc(QWidget *parent) :
  QMainWindow(parent),
  settings_model_(this),
  ui(new Ui::tpcc)
{
  qRegisterMetaType<Variant>("Variant");

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

  ui->tableParams->setModel(&settings_model_);
  ui->tableParams->setItemDelegate(&params_delegate_);
  ui->tableParams->horizontalHeader()->setStretchLastSection(true);
  ui->tableParams->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  connect(&settings_model_, SIGNAL(settings_changed()), this, SLOT(table_changed()));

  thread_classify_.set_refresh_frequency(5);
  ui->comboGroup->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  QTimer::singleShot(1000, this, SLOT(loadSettings()));
}

tpcc::~tpcc()
{
  CustomLogger::closeLogger();
  delete ui;
}

void tpcc::table_changed()
{
  NMX::Settings parameters = settings_model_.get_settings();
  reader_->set_parameters(parameters);
  event_viewer_->refresh_event();
}

void tpcc::display_params()
{
  auto parameters = reader_->get_parameters();
  settings_model_.update(parameters);
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

  reader_ = std::make_shared<NMX::FileAPV>(fileName.toStdString());

  event_viewer_->set_new_source(reader_);

  int evt_count = reader_->event_count();

  QSettings settings;
  settings.beginGroup("Program");
  if (evt_count)
  {
    ui->pushOpen->setText("  " + fileName);
    settings.setValue("recent_file", fileName);
    event_viewer_->refresh_event();
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

  if (enable) {
    ui->tableParams->setEditTriggers(QAbstractItemView::AllEditTriggers);
  } else {
    ui->tableParams->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }

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

  thread_classify_.go(reader_);
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
  auto name = ui->comboGroup->currentText();

  QProgressDialog* progress = new QProgressDialog("Loading analysis '" + name + "'",
                                                  "Abort Copy",
                                                  0, 100, this);
  progress->setWindowModality(Qt::WindowModal);
  progress->show();
  progress->setValue(0);

//  for (int i = 0; i < numFiles; i++)
//  {
//    progress.setValue(i);
//    if (progress.wasCanceled())
//      break;
//    //... copy one file
//  }

  reader_->load_analysis(name.toStdString());
  progress->setValue(100);

  double percent = double(reader_->num_analyzed()) / double(reader_->event_count()) * 100;

  display_params();
  ui->pushShowParams->setChecked(reader_->num_analyzed() == 0);
  on_pushShowParams_clicked();

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

void tpcc::on_pushShowParams_clicked()
{
  ui->widgetParams->setVisible(ui->pushShowParams->isChecked());
}
