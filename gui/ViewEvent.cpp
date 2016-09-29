#include <QSettings>
#include "ViewEvent.h"
#include "ui_ViewEvent.h"
#include "CustomLogger.h"
#include "qt_util.h"

ViewEvent::ViewEvent(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ViewEvent)
{
  ui->setupUi(this);

  ui->comboPlanes->addItem("X");
  ui->comboPlanes->addItem("Y");
  ui->comboPlanes->addItem("X & Y");

  ui->tableValues->verticalHeader()->hide();
  ui->tableValues->setColumnCount(2);
  ui->tableValues->setHorizontalHeaderLabels({"parameter", "value"});
  ui->tableValues->setSelectionMode(QAbstractItemView::NoSelection);
  ui->tableValues->setEditTriggers(QTableView::NoEditTriggers);
  ui->tableValues->horizontalHeader()->setStretchLastSection(true);
  ui->tableValues->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  NMX::Record rec;
  rec.analyze();
  for (auto &name : rec.point_categories())
    ui->comboOverlay->addItem(QString::fromStdString(name));
  ui->comboOverlay->addItem("none");

  NMX::Event evt;
  evt.analyze();
  for (auto &name : evt.projection_categories())
    ui->comboProjection->addItem(QString::fromStdString(name));
  ui->comboProjection->addItem("none");

  ui->eventX->set_title("X plane");
  ui->eventY->set_title("Y plane");

  loadSettings();

  ui->eventX->set_overlay_type(ui->comboOverlay->currentText());
  ui->eventY->set_overlay_type(ui->comboOverlay->currentText());

  ui->eventX->set_show_raw(ui->checkRaw->isChecked());
  ui->eventY->set_show_raw(ui->checkRaw->isChecked());

  ui->plotProjection->set_scale_type("Linear");
  ui->plotProjection->set_plot_style("Step center");
  ui->plotProjection->set_visible_options(ShowOptions::zoom | ShowOptions::thickness | ShowOptions::grid | ShowOptions::save);

  on_comboPlanes_currentIndexChanged("");
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
  ui->comboPlanes->setEnabled(en);
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
  ui->comboPlanes->setCurrentText(settings.value("show_planes", "X & Y").toString());
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
  settings.setValue("show_planes", ui->comboPlanes->currentText());
}


void ViewEvent::clear()
{
  ui->eventX->clear();
  ui->eventY->clear();

  ui->plotProjection->reset_scales();
  ui->plotProjection->clearGraphs();
  ui->plotProjection->clearExtras();
  ui->plotProjection->replot_markers();
  ui->plotProjection->rescale();
  ui->plotProjection->redraw();
}

void ViewEvent::on_spinEventIdx_valueChanged(int /*arg1*/)
{
  int evt_count {0};
  if (reader_)
    evt_count = reader_->event_count();

  int idx = ui->spinEventIdx->value();
  size_t evt_idx = 0;
  if (idx <= int(indices_.size()))
    evt_idx = indices_[idx-1];

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
  plot_current_event();
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
  if ((idx > 0) && (idx <= int(indices_.size())))
    evt_idx = indices_[idx-1];

  if (!reader_ || (evt_idx >= reader_->event_count()))
  {
    clear();
    return;
  }


  NMX::Event evt = reader_->get_event_with_metrics(evt_idx);

  if (evt_idx >= reader_->num_analyzed())
  {
    evt.set_parameters(params_);
    evt.analyze();
  }

  ui->eventX->set_suppress_negatives(ui->checkNoneg->isChecked());
  ui->eventY->set_suppress_negatives(ui->checkNoneg->isChecked());

  ui->eventX->display_record(evt.x());
  ui->eventY->display_record(evt.y());

  auto metrics = evt.metrics();
  if (ui->comboPlanes->currentText() == "X")
    metrics = evt.x().metrics();
  else if (ui->comboPlanes->currentText() == "Y")
    metrics = evt.y().metrics();

  ui->tableValues->clearContents();
  ui->tableValues->setRowCount(metrics.size());
  int i = 0;
  for (auto &a : metrics)
  {
    add_to_table(ui->tableValues, i, 0, a.first);
    add_to_table(ui->tableValues, i, 1, a.second.value.to_string());
    i++;
  }

  display_projection(evt);
}

void ViewEvent::set_indices(std::set<size_t> indices)
{
  indices_.clear();
  std::copy( indices.begin(), indices.end(), std::inserter( indices_, indices_.end() ) );

  size_t evt_count {0};

  if (reader_)
    evt_count = reader_->event_count();

  if ((evt_count > 0) && (reader_->num_analyzed() == 0))
    for (size_t i=0; i < evt_count; ++i)
      indices_.push_back(i);

  if (indices_.size() < evt_count)
    evt_count = indices_.size();

  ui->spinEventIdx->setEnabled(evt_count > 0);
  ui->spinEventIdx->setRange(1, evt_count);
  ui->spinEventIdx->setValue(1);
  on_spinEventIdx_valueChanged(0);
}

void ViewEvent::display_projection(NMX::Event &evt)
{
  std::map<double, double> minima, maxima;

  ui->plotProjection->clearGraphs();

  QVector<double> x, y;

  for (auto &pts : evt.get_projection(ui->comboProjection->currentText().toStdString()))
  {
    double xx = pts.first;
    double yy = pts.second;

    x.push_back(xx);
    y.push_back(yy);

    if (!minima.count(xx) || (minima[xx] > yy))
      minima[xx] = yy;
    if (!maxima.count(xx) || (maxima[xx] < yy))
      maxima[xx] = yy;
  }

  AppearanceProfile profile;
  profile.default_pen = QPen(Qt::darkRed, 2);
  ui->plotProjection->addGraph(x, y, profile, 8);

  ui->plotProjection->setLabels("position", "value");
  ui->plotProjection->setYBounds(minima, maxima);

  ui->plotProjection->setTitle(ui->comboProjection->currentText());
  ui->plotProjection->replot_markers();
  ui->plotProjection->redraw();
}


void ViewEvent::on_comboPlanes_currentIndexChanged(const QString &arg1)
{
  ui->eventX->setVisible(ui->comboPlanes->currentText().contains("X"));
  ui->eventY->setVisible(ui->comboPlanes->currentText().contains("Y"));
  plot_current_event();
}
