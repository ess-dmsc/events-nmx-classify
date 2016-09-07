#include <QSettings>
#include "ViewEvent.h"
#include "ui_ViewEvent.h"
#include "CustomLogger.h"


ViewEvent::ViewEvent(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ViewEvent)
{
  ui->setupUi(this);

  NMX::Record rec;
  rec.analyze();
  for (auto &name : rec.point_categories())
    ui->comboOverlay->addItem(QString::fromStdString(name));
  ui->comboOverlay->addItem("none");

  ui->comboProjection->addItem("none");
  ui->comboProjection->addItem("Integral");

  ui->eventX->set_title("X strips");
  ui->eventY->set_title("Y strips");

  loadSettings();

  ui->eventX->set_overlay_type(ui->comboOverlay->currentText());
  ui->eventY->set_overlay_type(ui->comboOverlay->currentText());

  ui->eventX->set_projection_type(ui->comboProjection->currentText());
  ui->eventY->set_projection_type(ui->comboProjection->currentText());

  ui->eventX->set_show_raw(ui->checkRaw->isChecked());
  ui->eventY->set_show_raw(ui->checkRaw->isChecked());
}

ViewEvent::~ViewEvent()
{
  saveSettings();
  delete ui;
}

void ViewEvent::enableIO(bool enable)
{
  bool en = reader_ && reader_->event_count() && enable;
  ui->spinEventIdx->setEnabled(en);
  ui->checkNoneg->setEnabled(en);
}

void ViewEvent::set_new_source(std::shared_ptr<NMX::FileHDF5> r)
{
  reader_ = r;

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

    std::set<size_t> indices;
    for (int i=0; i < evt_count; ++i)
      indices.insert(i);
    set_indices(indices);
  }
  else
    clear();
}

void ViewEvent::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  ui->checkNoneg->setChecked(settings.value("noneg", false).toBool());
  ui->checkRaw->setChecked(settings.value("show_raw", true).toBool());
  ui->comboOverlay->setCurrentText(settings.value("overlay").toString());
  ui->comboProjection->setCurrentText(settings.value("projection").toString());
}

void ViewEvent::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("noneg", ui->checkNoneg->isChecked());
  settings.setValue("show_raw", ui->checkRaw->isChecked());
  settings.setValue("current_idx", ui->spinEventIdx->value());
  settings.setValue("overlay", ui->comboOverlay->currentText());
  settings.setValue("projection", ui->comboProjection->currentText());
}


void ViewEvent::clear()
{
  ui->eventX->clear();
  ui->eventY->clear();
}

void ViewEvent::on_spinEventIdx_valueChanged(int /*arg1*/)
{
  int evt_count {0};
  if (reader_)
    evt_count = reader_->event_count();

  int idx = ui->spinEventIdx->value();
  size_t evt_idx = 0;
  if (idx < int(indices_.size()))
    evt_idx = indices_[idx] + 1;

  ui->labelOfTotal->setText(" of " + QString::number(indices_.size())
                            + "   [" + QString::number(evt_idx) + "]");
  plot_current_event();
}

void ViewEvent::on_checkNoneg_clicked()
{
  plot_current_event();
}

void ViewEvent::on_comboOverlay_currentIndexChanged(const QString &/*arg1*/)
{
  ui->eventX->set_overlay_type(ui->comboOverlay->currentText());
  ui->eventY->set_overlay_type(ui->comboOverlay->currentText());
}


void ViewEvent::on_comboProjection_activated(const QString &/*arg1*/)
{
  ui->eventX->set_projection_type(ui->comboProjection->currentText());
  ui->eventY->set_projection_type(ui->comboProjection->currentText());
}

void ViewEvent::on_checkRaw_clicked()
{
  ui->eventX->set_show_raw(ui->checkRaw->isChecked());
  ui->eventY->set_show_raw(ui->checkRaw->isChecked());
}

void ViewEvent::set_params(NMX::Settings params)
{
  params_ = params;
  plot_current_event();
}


void ViewEvent::plot_current_event()
{
  int idx = ui->spinEventIdx->value();
  size_t evt_idx = 0;
  if (idx < int(indices_.size()))
    evt_idx = indices_[idx] + 1;

  if (!reader_ || (evt_idx < 1) || (evt_idx > reader_->event_count()))
  {
    clear();
    return;
  }


  NMX::Event evt = reader_->get_event(evt_idx - 1);

  if (ui->checkNoneg->isChecked())
    evt = evt.suppress_negatives();

  evt.set_parameters(params_);
  evt.analyze();

  ui->eventX->display_record(evt.x());
  ui->eventY->display_record(evt.y());
}

void ViewEvent::set_indices(std::set<size_t> indices)
{
  indices_.clear();
  std::copy( indices.begin(), indices.end(), std::inserter( indices_, indices_.end() ) );

  size_t evt_count {0};

  if (reader_)
    evt_count = reader_->event_count();

  if (indices.size() < evt_count)
    evt_count = indices_.size();

  ui->spinEventIdx->setEnabled(evt_count > 0);
  ui->spinEventIdx->setRange(1, evt_count);
  ui->spinEventIdx->setValue(1);
  on_spinEventIdx_valueChanged(0);
}



