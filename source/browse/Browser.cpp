#include <QSettings>

#include "Browser.h"
#include "ui_Browser.h"

#include "CustomLogger.h"
#include "Variant.h"
#include "ExceptionUtil.h"

Browser::Browser(QWidget *parent) :
  QMainWindow(parent),
  settings_model_(this),
  ui(new Ui::Browser)
{
  qRegisterMetaType<Variant>("Variant");

  CustomLogger::initLogger();
  ui->setupUi(this);

  event_viewer_ = new ViewEvent();
  ui->tabWidget->addTab(event_viewer_, "Event viewer");
  connect(this, SIGNAL(enableIO(bool)), event_viewer_, SLOT(enableIO(bool)));
  connect(event_viewer_, SIGNAL(planes_selected()), this, SLOT(display_params()));

  analyzer_ = new Analyzer();
  ui->tabWidget->addTab(analyzer_, "Analyzer");
  connect(analyzer_, SIGNAL(toggleIO(bool)), this, SLOT(toggleIO(bool)));
  connect(this, SIGNAL(enableIO(bool)), analyzer_, SLOT(enableIO(bool)));

  review_ = new AggregateReview();
  ui->tabWidget->addTab(review_, "Review");
  connect(review_, SIGNAL(digDownTown(QString,QString,QString)),
          this, SLOT(digDownTown(QString,QString,QString)));

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

Browser::~Browser()
{
  CustomLogger::closeLogger();
  delete ui;
}

void Browser::table_changed()
{
  NMX::Settings parameters = settings_model_.get_settings();
  reader_->set_parameters(parameters);
  event_viewer_->refresh_event();
}

void Browser::display_params()
{
  NMX::Settings settings;
  if (reader_)
  {
    for (auto s : reader_->parameters().data())
    {
      if ((event_viewer_->x_visible() && QString::fromStdString(s.first).contains("x.")) ||
          (event_viewer_->y_visible() && QString::fromStdString(s.first).contains("y.")))
        settings.set(s.first, s.second);
    }
  }
  settings_model_.update(settings);
}

void Browser::closeEvent(QCloseEvent *event)
{
  saveSettings();
  event->accept();
}

void Browser::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  QRect myrect = settings.value("position",QRect(20,20,1234,650)).toRect();
  setGeometry(myrect);
  data_directory_ = settings.value("data_directory", "").toString();

  QString fileName = settings.value("recent_file").toString();

  open_file(fileName);
}

void Browser::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("position", this->geometry());
  settings.setValue("data_directory", data_directory_);
}

bool Browser::open_file(QString fileName)
{
  data_directory_ = path_of_file(fileName);

  try
  {
    reader_ = std::make_shared<NMX::FileAPV>(fileName.toStdString());
  }
  catch (...)
  {
    printException();
    return false;
  }

  if (reader_->has_raw())
    reader_->open_raw();

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

  if (!reader_->analyses().empty())
    ui->comboGroup->setCurrentText(QString::fromStdString(reader_->analyses().front()));
  on_comboGroup_activated("");

  return (evt_count > 0);
}

void Browser::toggleIO(bool enable)
{
  auto names = reader_->analyses();

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

void Browser::on_pushStop_clicked()
{
  thread_classify_.terminate();
}

void Browser::on_pushStart_clicked()
{
  if (!reader_|| !reader_->event_count())
    return;

  ui->pushStop->setEnabled(true);
  toggleIO(false);

  thread_classify_.go(reader_);
}

void Browser::update_progress(double percent_done)
{
  ui->progressBar->setValue(percent_done);
}


void Browser::run_complete()
{
  toggleIO(true);
  ui->pushStop->setEnabled(false);

//  reader_->save_analysis();
  analyzer_->set_new_source(reader_);
}

void Browser::on_comboGroup_activated(const QString& /*arg1*/)
{
  toggleIO(false);
  auto name = ui->comboGroup->currentText();

  reader_->load_analysis(name.toStdString());

  double percent = double(reader_->num_analyzed()) / double(reader_->event_count()) * 100;

  display_params();
  ui->pushShowParams->setChecked(reader_->num_analyzed() == 0);
  on_pushShowParams_clicked();

  ui->progressBar->setValue(percent);
  analyzer_->set_new_source(reader_);
  toggleIO(true);
}

void Browser::on_pushNewGroup_clicked()
{
  bool ok = false;
  QString text = QInputDialog::getText(this, "New analysis group",
                                       "Group name:", QLineEdit::Normal,
                                       "", &ok);
  if (!ok || text.isEmpty())
    return;

  //must not have slashes!
  //must not already exist

  for (auto &name : reader_->analyses())
    if (name == text.toStdString())
      return;

  reader_->create_analysis(text.toStdString());

  populate_combo();

  ui->comboGroup->setCurrentText(text);
  on_comboGroup_activated(text);
}

void Browser::populate_combo()
{
  ui->comboGroup->clear();
  for (auto &name : reader_->analyses())
    ui->comboGroup->addItem(QString::fromStdString(name));
  toggleIO(true);
}

void Browser::on_pushDeleteGroup_clicked()
{
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, "Delete analysis?",
                                "Delete analysis '" + ui->comboGroup->currentText() + "'?",
                                QMessageBox::Yes|QMessageBox::No);
  if (reply == QMessageBox::Yes)
  {
    reader_->delete_analysis(ui->comboGroup->currentText().toStdString());
    populate_combo();
    on_comboGroup_activated("");
  }
}

void Browser::on_pushOpen_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Load TPC data", data_directory_, "HDF5 (*.h5)");
  if (!validateFile(this, fileName, false))
    return;

  open_file(fileName);
}

void Browser::on_pushShowParams_clicked()
{
  ui->widgetParams->setVisible(ui->pushShowParams->isChecked());
}

void Browser::digDownTown(QString dset, QString metric, QString file)
{
  if (!ui->pushOpen->text().contains(file))
    open_file(data_directory_+"/"+file+".h5");
  if (ui->comboGroup->currentText() != dset)
  {
    ui->comboGroup->setCurrentText(dset);
    on_comboGroup_activated("");
  }
  analyzer_->set_metric_z(metric);
  ui->tabWidget->setCurrentWidget(analyzer_);
}
