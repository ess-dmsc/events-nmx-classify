#include <QSettings>

#include "Browser.h"
#include "ui_Browser.h"

#include "CustomLogger.h"
#include "Variant.h"
#include "ExceptionUtil.h"
#include <QDialog>
#include <QTableWidget>
#include <QGridLayout>

Browser::Browser(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Browser)
{
  qRegisterMetaType<Variant>("Variant");

  CustomLogger::initLogger();
  ui->setupUi(this);

  event_viewer_ = new ViewEvent();
  ui->tabWidget->addTab(event_viewer_, "Event viewer");

  analyzer_ = new Analyzer();
  ui->tabWidget->addTab(analyzer_, "Dataset metrics");
  connect(analyzer_, SIGNAL(select_indices(std::set<size_t>)),
          event_viewer_, SLOT(set_indices(std::set<size_t>)));

  review_ = new AggregateReview();
  ui->tabWidget->addTab(review_, "Aggregate hists");
  connect(review_, SIGNAL(digDownTown(QString,QString,QString)),
          this, SLOT(digDownTown(QString,QString,QString)));

  ui->comboGroup->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  QTimer::singleShot(1000, this, SLOT(loadSettings()));
}

Browser::~Browser()
{
  CustomLogger::closeLogger();
  delete ui;
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

void Browser::open_file(QString fileName)
{
  data_directory_ = path_of_file(fileName);

  try
  {
    reader_ = std::make_shared<NMX::FileAPV>(fileName.toStdString(), H5CC::Access::rw_existing);
  }
  catch (...)
  {
    printException();
    ERR << "<nmx_browser> could not open " << fileName.toStdString();
    return;
  }

  if (reader_->has_raw())
    reader_->open_raw();

  int evt_count = reader_->event_count();

  QSettings settings;
  settings.beginGroup("Program");
  if (evt_count)
  {
    ui->pushOpen->setText("  " + fileName);
    settings.setValue("recent_file", fileName);
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
}

void Browser::toggleIO(bool enable)
{
  auto names = reader_->analyses();

  ui->pushOpen->setEnabled(enable);

  ui->comboGroup->setEnabled(enable && (names.size() > 0));

  ui->pushNewGroup->setEnabled(enable && reader_ && reader_->event_count());
  ui->pushDeleteGroup->setEnabled(enable && reader_ && names.size());

  emit enableIO(enable);
}

void Browser::on_comboGroup_activated(const QString& /*arg1*/)
{
  toggleIO(false);
  auto name = ui->comboGroup->currentText();

  reader_->load_analysis(name.toStdString());

  double percent = double(reader_->num_analyzed()) / double(reader_->event_count()) * 100;
  ui->labelPercent->setText(QString::number(percent) + "% complete ");

  analyzer_->set_new_source(reader_);
  event_viewer_->set_new_source(reader_);

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

void Browser::digDownTown(QString dset, QString metric, QString file)
{
  if (!ui->pushOpen->text().contains(file))
    open_file(file);
  if (ui->comboGroup->currentText() != dset)
  {
    ui->comboGroup->setCurrentText(dset);
    on_comboGroup_activated("");
  }
  analyzer_->set_metric_z(metric);
  ui->tabWidget->setCurrentWidget(analyzer_);
}

void Browser::on_pushMetricsGlossary_clicked()
{
  if (!reader_)
    return;

  QTableWidget* table = new QTableWidget;
  table->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  table->horizontalHeader()->setStretchLastSection(true);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionMode(QAbstractItemView::NoSelection);

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Metrics glossary");
  dialog->setSizeGripEnabled(true);
  dialog->resize(800, 400);
  QGridLayout* layout = new QGridLayout;
  layout->setMargin(3);

  layout->addWidget(table, 0,0, -1,-1);
  dialog->setLayout(layout);

  auto metrics = reader_->metrics();
  table->setRowCount(metrics.size());
  table->setColumnCount(2);
  table->setHorizontalHeaderLabels(QStringList({"Metric", "Description"}));

  int i=0;
  for (auto m : metrics)
  {
    auto metric = reader_->get_metric(m, false);
    table->setItem(i,0,new QTableWidgetItem(QString::fromStdString(m)));
    table->setItem(i,1,new QTableWidgetItem(QString::fromStdString(metric.description())));
    i++;
  }

  dialog->exec();
}
