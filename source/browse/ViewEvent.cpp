#include <QSettings>
#include "ViewEvent.h"
#include "ui_ViewEvent.h"
#include "CustomLogger.h"
#include <QTableWidgetItem>
#include "ViewRecord.h"


ViewEvent::ViewEvent(QWidget *parent) :
  QWidget(parent),
  params_model_(this),
  ui(new Ui::ViewEvent)
{
  ui->setupUi(this);
  ui->eventX->set_title("X plane");
  ui->eventY->set_title("Y plane");
  ui->plotProjection->setScaleType("Linear");
  ui->plotProjection->setPlotStyle("Step center");
  ui->comboPlanes->addItem("X");
  ui->comboPlanes->addItem("Y");
  ui->comboPlanes->addItem("X & Y");

  ui->tableParams->setModel(&params_model_);
  ui->tableParams->setItemDelegate(&params_delegate_);
  ui->tableParams->horizontalHeader()->setStretchLastSection(true);
  ui->tableParams->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(&params_model_, SIGNAL(settings_changed()), this, SLOT(parametersModified()));

  ui->tableMetrics->setModel(&metrics_model_);
  ui->tableMetrics->horizontalHeader()->setStretchLastSection(true);
  ui->tableMetrics->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableMetrics->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tableMetrics->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->tableMetrics->setSelectionBehavior(QAbstractItemView::SelectRows);
  connect(ui->searchBoxMetrics, SIGNAL(selectionChanged()), this, SLOT(metricsFilterChanged()));

  //record display options
  connect(ui->color1, SIGNAL(colorChanged(QColor)), this, SLOT(recordOptionsChanged()));
  connect(ui->push1x, SIGNAL(selectionChanged(QString)), this, SLOT(recordOptionsChanged()));
  connect(ui->push1y, SIGNAL(selectionChanged(QString)), this, SLOT(recordOptionsChanged()));
  connect(ui->color2, SIGNAL(colorChanged(QColor)), this, SLOT(recordOptionsChanged()));
  connect(ui->push2x, SIGNAL(selectionChanged(QString)), this, SLOT(recordOptionsChanged()));
  connect(ui->push2y, SIGNAL(selectionChanged(QString)), this, SLOT(recordOptionsChanged()));
  connect(ui->colorOverlay, SIGNAL(colorChanged(QColor)), this, SLOT(recordOptionsChanged()));
  connect(ui->comboPlot, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(recordOptionsChanged()));
  connect(ui->comboOverlay, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(recordOptionsChanged()));
  connect(ui->comboProjection, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(plotProjection()));

  populateCombos();

  on_comboPlanes_currentIndexChanged("");
  recordOptionsChanged();
}

ViewEvent::~ViewEvent()
{
  saveSettings();
  delete ui;
}

void ViewEvent::populateCombos()
{
  NMX::Event evt;
  if (reader_)
    evt.set_parameters(reader_->parameters());
  evt.analyze();

  ui->comboProjection->blockSignals(true);
  ui->comboPlot->blockSignals(true);
  ui->comboOverlay->blockSignals(true);

  ui->comboProjection->clear();
  ui->comboPlot->clear();
  ui->comboOverlay->clear();

  for (auto &name : evt.projection_categories())
    ui->comboProjection->addItem(QString::fromStdString(name));
  ui->comboProjection->addItem("none");

  NMX::Record rec = evt.x();
  for (auto &name : rec.point_categories())
  {
    ui->comboPlot->addItem(QString::fromStdString(name));
    ui->comboOverlay->addItem(QString::fromStdString(name));
  }
  ui->comboOverlay->addItem("none");
  ui->comboPlot->addItem("none");
  ui->comboPlot->addItem("everything");

  loadSettings();

  ui->comboProjection->blockSignals(false);
  ui->comboPlot->blockSignals(false);
  ui->comboOverlay->blockSignals(false);
}

void ViewEvent::parametersModified()
{
  auto all_params = reader_->parameters();
  for (auto p : params_model_.get_settings().data())
    all_params.set(p.first, p.second);
  reader_->set_parameters(all_params);
  saveSettings();
  populateCombos();
  plot_current_event();
}

void ViewEvent::update_parameters_model()
{
  NMX::Settings settings;
  bool x_visible = ui->comboPlanes->currentText().contains("X");
  bool y_visible = ui->comboPlanes->currentText().contains("Y");
  if (reader_)
  {
    for (auto s : reader_->parameters().data())
    {
      if ((x_visible && QString::fromStdString(s.first).contains("x.")) ||
          (y_visible && QString::fromStdString(s.first).contains("y.")))
        settings.set(s.first, s.second);
    }
  }
  params_model_.update(settings);
}

void ViewEvent::set_new_source(std::shared_ptr<NMX::File> r)
{
  reader_ = r;

  int evt_count {0};

  if (reader_)
    evt_count = reader_->event_count();

  ui->spinEventIdx->setEnabled(evt_count > 0);
  ui->spinEventIdx->setRange(1, evt_count);
  ui->spinEventIdx->setValue(1);

  if (reader_ && !reader_->num_analyzed())
    ui->tableParams->setEditTriggers(QAbstractItemView::AllEditTriggers);
  else
    ui->tableParams->setEditTriggers(QAbstractItemView::NoEditTriggers);

  if (evt_count > 0)
  {
    std::set<size_t> indices;
    for (int i=0; i < evt_count; ++i)
      indices.insert(i);
    set_indices(indices);
  }
  else
    clear();

  saveSettings();
  populateCombos();

  if (reader_->num_analyzed() != 0)
  {
    ui->tableParams->setVisible(false);
    on_pushShowParams_clicked();
  }
  update_parameters_model();

  plot_current_event();
}

void ViewEvent::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.beginGroup("Event");
  ui->comboPlot->setCurrentText(settings.value("plot", "everything").toString());
  ui->comboOverlay->setCurrentText(settings.value("overlay", "maxima").toString());
  ui->colorOverlay->setColor(QColor(settings.value("color_overlay", QColor(Qt::red).name()).toString()));
  ui->comboProjection->setCurrentText(settings.value("projection", "none").toString());
  ui->push1x->setText(settings.value("point1x").toString());
  ui->push2x->setText(settings.value("point2x").toString());
  ui->color1->setColor(QColor(settings.value("color1", QColor(Qt::yellow).name()).toString()));
  ui->push1y->setText(settings.value("point1y").toString());
  ui->push2y->setText(settings.value("point2y").toString());
  ui->color2->setColor(QColor(settings.value("color2", QColor(Qt::magenta).name()).toString()));
  ui->comboPlanes->setCurrentText(settings.value("show_planes", "X & Y").toString());
  ui->searchBoxMetrics->setFilter(settings.value("filter").toString());
}

void ViewEvent::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.beginGroup("Event");
  settings.setValue("current_idx", ui->spinEventIdx->value());
  settings.setValue("plot", ui->comboPlot->currentText());
  settings.setValue("overlay", ui->comboOverlay->currentText());
  settings.setValue("color_overlay", ui->colorOverlay->color().name());
  settings.setValue("projection", ui->comboProjection->currentText());
  settings.setValue("point1x", ui->push1x->text());
  settings.setValue("point2x", ui->push2x->text());
  settings.setValue("color1", ui->color1->color().name());
  settings.setValue("point1y", ui->push1y->text());
  settings.setValue("point2y", ui->push2y->text());
  settings.setValue("color2", ui->color2->color().name());
  settings.setValue("show_planes", ui->comboPlanes->currentText());
  settings.setValue("filter", ui->searchBoxMetrics->filter());
}


void ViewEvent::clear()
{
  event_ = NMX::Event();

  ui->eventX->clear();
  ui->eventY->clear();

  ui->plotProjection->clearAll();
  ui->plotProjection->zoomOut();
}

void ViewEvent::on_spinEventIdx_valueChanged(int /*arg1*/)
{
  plot_current_event();
}

void ViewEvent::recordOptionsChanged()
{
  set_record_options();
  refresh_record_plots();
}

void ViewEvent::plot_current_event()
{
  int evt_count {0};
  if (reader_)
    evt_count = reader_->event_count();

  int idx = ui->spinEventIdx->value();
  size_t evt_idx = 0;
  if ((idx > 0) && (idx <= int(indices_.size())))
    evt_idx = indices_.at(idx-1);

  ui->labelOfFiltered->setText(" of " + QString::number(indices_.size()));
  if (evt_count)
    ui->labelTotal->setText("[" + QString::number(evt_idx) + "/" + QString::number(evt_count-1) + "]");
  else
    ui->labelTotal->setText("");

  if (!reader_ || (evt_idx >= evt_count))
  {
    clear();
    return;
  }

  event_ = reader_->get_event(evt_idx);

  set_record_options();

  ui->eventX->display_record(event_.x());
  ui->eventY->display_record(event_.y());

  if (ui->comboPlanes->currentText() == "X")
    ui->searchBoxMetrics->setList(getMetricsList(event_.x().metrics(), false));
  else if (ui->comboPlanes->currentText() == "Y")
    ui->searchBoxMetrics->setList(getMetricsList(event_.y().metrics(), false));
  else
    ui->searchBoxMetrics->setList(getMetricsList(event_.metrics(), false));

  auto coord_metrics = event_.x().metrics().with_suffix("_c", false);
  auto coords_x = getMetricsList(coord_metrics.with_prefix("strips_", false), true);
  ui->push1x->setList(coords_x);
  ui->push2x->setList(coords_x);

  auto coords_y = getMetricsList(coord_metrics.with_prefix("timebins_", false), true);
  ui->push1y->setList(coords_y);
  ui->push2y->setList(coords_y);

  plotProjection();
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

  auto current_val = ui->spinEventIdx->value();
  ui->spinEventIdx->setEnabled(indices_.size() > 0);
  ui->spinEventIdx->setRange(1, indices_.size());
  if (current_val <= indices_.size())
    ui->spinEventIdx->setValue(current_val);
  else
    ui->spinEventIdx->setValue(1);

  plot_current_event();
}

void ViewEvent::plotProjection()
{
  bool visible = (ui->comboProjection->currentText() != "none");
  ui->plotProjection->setVisible(visible);
  if (!visible)
    return;

  ui->plotProjection->clearAll();

//  QPlot::Appearance profile;
//  profile.default_pen = QPen(Qt::darkRed, 2);
  ui->plotProjection->addGraph(event_.get_projection(ui->comboProjection->currentText().toStdString()),
                               QPen(Qt::darkRed, 2));
  ui->plotProjection->setAxisLabels("position", "value");
  ui->plotProjection->setTitle(ui->comboProjection->currentText());
  ui->plotProjection->zoomOut();
}

void ViewEvent::on_comboPlanes_currentIndexChanged(const QString&)
{
  ui->eventX->setVisible(ui->comboPlanes->currentText().contains("X"));
  ui->eventY->setVisible(ui->comboPlanes->currentText().contains("Y"));
  plot_current_event();
  update_parameters_model();
}

void ViewEvent::metricsFilterChanged()
{
  auto metrics = event_.metrics();
  if (ui->comboPlanes->currentText() == "X")
    metrics = event_.x().metrics();
  else if (ui->comboPlanes->currentText() == "Y")
    metrics = event_.y().metrics();

  NMX::MetricSet final;
  for (auto name : ui->searchBoxMetrics->selection())
    final.set(name.toStdString(), metrics.get(name.toStdString()));

  metrics_model_.set_metrics(final);
}

void ViewEvent::on_pushShowParams_clicked()
{
  ui->tableParams->setVisible(!ui->tableParams->isVisible());

  if (ui->tableParams->isVisible())
  {
    ui->pushShowParams->setIcon(QPixmap(":/icons/oxy/16/down.png"));
    ui->pushShowParams->setText("   Hide analysis parameters");
  }
  else
  {
    ui->pushShowParams->setIcon(QPixmap(":/icons/oxy/16/up.png"));
    ui->pushShowParams->setText("   Show analysis parameters");
  }
}

void ViewEvent::set_record_options()
{
  QVector<PointMetrics> pmetrics;
  PointMetrics p;

  p.x_metric = ui->push1x->text().toStdString();
  p.y_metric = ui->push1y->text().toStdString();
  p.color = ui->color1->color();
  pmetrics.push_back(p);

  p.x_metric = ui->push2x->text().toStdString();
  p.y_metric = ui->push2y->text().toStdString();
  p.color = ui->color2->color();
  pmetrics.push_back(p);

  ui->eventX->set_plot_type(ui->comboPlot->currentText());
  ui->eventY->set_plot_type(ui->comboPlot->currentText());

  ui->eventX->set_overlay_type(ui->comboOverlay->currentText());
  ui->eventY->set_overlay_type(ui->comboOverlay->currentText());

  ui->eventX->set_overlay_color(ui->colorOverlay->color());
  ui->eventY->set_overlay_color(ui->colorOverlay->color());

  ui->eventX->set_point_metrics(pmetrics);
  ui->eventY->set_point_metrics(pmetrics);
}

void ViewEvent::refresh_record_plots()
{
  if (ui->eventX->isVisible())
    ui->eventX->refresh();
  if (ui->eventY->isVisible())
    ui->eventY->refresh();
}

QStringList ViewEvent::getMetricsList(NMX::MetricSet metric_set, bool include_none)
{
  QStringList list;
  for (auto &m : metric_set.data())
    list.push_back(QString::fromStdString(m.first));
  if (include_none)
    list.push_back("none");
  return list;
}
