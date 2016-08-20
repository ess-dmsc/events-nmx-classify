#include <QSettings>
#include "event_viewer.h"
#include "ui_event_viewer.h"
#include "CustomLogger.h"


EventViewer::EventViewer(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::EventViewer)
{
  ui->setupUi(this);

  ui->comboOverlay->addItem("none");
  ui->comboOverlay->addItem("Maxima");
  ui->comboOverlay->addItem("VMM");

  ui->comboProjection->addItem("none");
  ui->comboProjection->addItem("Integral");
//  ui->comboProjection->addItem("Integral/bins");

  loadSettings();
}

EventViewer::~EventViewer()
{
  saveSettings();
  delete ui;
}

void EventViewer::enableIO(bool enable)
{
  bool en = reader_ && reader_->event_count() && enable;
  ui->spinEventIdx->setEnabled(en);
  ui->checkNoneg->setEnabled(en);
  ui->checkTrim->setEnabled(en);
  ui->comboOverlay->setEnabled(en);
  ui->comboProjection->setEnabled(en);
}

void EventViewer::set_new_source(std::shared_ptr<NMX::Reader> r, NMX::Dimensions x, NMX::Dimensions y)
{
  reader_ = r;
  xdims_ = x;
  ydims_ = y;

  int evt_count {0};

  if (reader_)
    evt_count = reader_->event_count();

  ui->labelOfTotal->setText(" of " + QString::number(evt_count));
  ui->spinEventIdx->setEnabled(evt_count > 0);
  ui->spinEventIdx->setRange(1, evt_count);
  ui->spinEventIdx->setValue(1);

  if (evt_count > 0)
  {
    QSettings settings;
    settings.beginGroup("Program");
    ui->spinEventIdx->setValue(settings.value("current_idx").toInt());
  }
  else
    clear();
}

void EventViewer::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  ui->checkTrim->setChecked(settings.value("trim").toBool());
  ui->checkNoneg->setChecked(settings.value("noneg").toBool());
  ui->comboOverlay->setCurrentText(settings.value("overlay").toString());
  ui->comboProjection->setCurrentText(settings.value("projection").toString());
}

void EventViewer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("trim", ui->checkTrim->isChecked());
  settings.setValue("noneg", ui->checkNoneg->isChecked());
  settings.setValue("current_idx", ui->spinEventIdx->value());
  settings.setValue("overlay", ui->comboOverlay->currentText());
  settings.setValue("projection", ui->comboProjection->currentText());
}


void EventViewer::clear()
{
  ui->eventX->clear();
  ui->eventY->clear();
}

void EventViewer::on_spinEventIdx_valueChanged(int /*arg1*/)
{
  plot_current_event();
}

void EventViewer::on_checkTrim_clicked()
{
  plot_current_event();
}

void EventViewer::on_checkNoneg_clicked()
{
  plot_current_event();
}

void EventViewer::on_comboOverlay_currentIndexChanged(const QString &/*arg1*/)
{
  plot_current_event();
}


void EventViewer::on_comboProjection_activated(const QString &/*arg1*/)
{
  plot_current_event();
}

void EventViewer::on_checkRaw_clicked()
{
  plot_current_event();
}

void EventViewer::plot_current_event()
{
  int idx = ui->spinEventIdx->value();

  if (!reader_ || (idx < 1) || (idx > reader_->event_count()))
  {
    clear();
    return;
  }

  bool trim = ui->checkTrim->isChecked();
  bool noneg = ui->checkNoneg->isChecked();

  NMX::Event evt = reader_->get_event(idx-1);

  if (noneg)
    evt = evt.suppress_negatives();

  evt.analyze();

  ui->eventX->display_record(evt.x(), xdims_, trim, ui->checkRaw->isChecked(), ui->comboOverlay->currentText());
  ui->eventY->display_record(evt.y(), ydims_, trim, ui->checkRaw->isChecked(), ui->comboOverlay->currentText());


  ui->eventX->display_projection(evt.x(), xdims_, ui->comboProjection->currentText());
  ui->eventY->display_projection(evt.y(), ydims_, ui->comboProjection->currentText());
}


