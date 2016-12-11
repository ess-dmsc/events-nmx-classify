#include <QSettings>
#include "ViewEvent.h"
#include "ui_ViewEvent.h"
#include "CustomLogger.h"
#include <QTableWidgetItem>
#include "ViewRecord.h"
#include "SearchList.h"

ViewEvent::ViewEvent(QWidget *parent) :
  QWidget(parent),
  params_model_(this),
  ui(new Ui::ViewEvent)
{
  ui->setupUi(this);

  connect(ui->searchBoxMetrics, SIGNAL(selectionChanged()), this, SLOT(metrics_selected()));

  connect(ui->color1, SIGNAL(clicked()), this, SLOT(clicked_color1()));
  connect(ui->color2, SIGNAL(clicked()), this, SLOT(clicked_color2()));
  connect(ui->colorOverlay, SIGNAL(clicked()), this, SLOT(clicked_colorOverlay()));

  ui->comboPlanes->addItem("X");
  ui->comboPlanes->addItem("Y");
  ui->comboPlanes->addItem("X & Y");

  ui->tableParams->setModel(&params_model_);
  ui->tableParams->setItemDelegate(&params_delegate_);
  ui->tableParams->horizontalHeader()->setStretchLastSection(true);
  ui->tableParams->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(&params_model_, SIGNAL(settings_changed()), this, SLOT(table_changed()));

  ui->tableMetrics->setModel(&metrics_model_);
  ui->tableMetrics->horizontalHeader()->setStretchLastSection(true);
  ui->tableMetrics->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableMetrics->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tableMetrics->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->tableMetrics->setSelectionBehavior(QAbstractItemView::SelectRows);

  ui->eventX->set_title("X plane");
  ui->eventY->set_title("Y plane");

  populateCombos(NMX::Event().parameters());

  ui->eventX->set_plot_type(ui->comboPlot->currentText());
  ui->eventY->set_plot_type(ui->comboPlot->currentText());

  ui->eventX->set_overlay_type(ui->comboOverlay->currentText());
  ui->eventY->set_overlay_type(ui->comboOverlay->currentText());

  set_point_metrics();

  ui->plotProjection->setScaleType("Linear");
  ui->plotProjection->setPlotStyle("Step center");
//  ui->plotProjection->set_visible_options(ShowOptions::style | ShowOptions::scale |
//                                          ShowOptions::labels | ShowOptions::thickness |
//                                          ShowOptions::grid | ShowOptions::save |
//                                          ShowOptions::zoom | ShowOptions::dither);

  on_comboPlanes_currentIndexChanged("");
  on_comboProjection_activated("");
}

void ViewEvent::set_point_metrics()
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

  ui->eventX->set_point_metrics(pmetrics);
  ui->eventY->set_point_metrics(pmetrics);
}


ViewEvent::~ViewEvent()
{
  saveSettings();
  delete ui;
}

void ViewEvent::populateCombos(const NMX::Settings &parameters)
{
  NMX::Event evt;
  evt.set_parameters(parameters);
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
  ui->eventX->set_overlay_color(ui->colorOverlay->color());
  ui->eventY->set_overlay_color(ui->colorOverlay->color());

  ui->comboProjection->blockSignals(false);
  ui->comboPlot->blockSignals(false);
  ui->comboOverlay->blockSignals(false);
}

void ViewEvent::table_changed()
{
  saveSettings();
  auto all_params = reader_->parameters();
  for (auto p : params_model_.get_settings().data())
    all_params.set(p.first, p.second);
  reader_->set_parameters(all_params);
  populateCombos(all_params);
  refresh_event();
}

void ViewEvent::display_params()
{
  NMX::Settings settings;
  if (reader_)
  {
    for (auto s : reader_->parameters().data())
    {
      if ((x_visible() && QString::fromStdString(s.first).contains("x.")) ||
          (y_visible() && QString::fromStdString(s.first).contains("y.")))
        settings.set(s.first, s.second);
    }
  }
  params_model_.update(settings);
}

void ViewEvent::enableIO(bool enable)
{
  bool en = reader_ && reader_->event_count() && enable;
  ui->spinEventIdx->setEnabled(en);
  ui->comboPlanes->setEnabled(en);

  bool editparams = enable && reader_ && !reader_->num_analyzed();;

  if (editparams) {
    ui->tableParams->setEditTriggers(QAbstractItemView::AllEditTriggers);
  } else {
    ui->tableParams->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }
}

void ViewEvent::set_new_source(std::shared_ptr<NMX::FileAPV> r)
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

  saveSettings();
  populateCombos(reader_->parameters());
  refresh_event();

  if (reader_->num_analyzed() == 0)
  {
    ui->tableParams->setVisible(false);
    on_pushShowParams_clicked();
  }
  display_params();
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
  int evt_count {0};
  if (reader_)
    evt_count = reader_->event_count();

  int idx = ui->spinEventIdx->value();
  size_t evt_idx = 0;
  if (idx <= int(indices_.size()))
    evt_idx = indices_[idx-1];

  ui->labelOfTotal->setText(" of " + QString::number(indices_.size())
                            + "   [" + QString::number(evt_idx) + "/" + QString::number(evt_count) + "]");
  plot_current_event();
}

void ViewEvent::on_comboPlot_currentIndexChanged(const QString &/*arg1*/)
{
  ui->eventX->set_plot_type(ui->comboPlot->currentText());
  ui->eventY->set_plot_type(ui->comboPlot->currentText());
}

void ViewEvent::on_comboOverlay_currentIndexChanged(const QString &/*arg1*/)
{
  ui->eventX->set_overlay_type(ui->comboOverlay->currentText());
  ui->eventY->set_overlay_type(ui->comboOverlay->currentText());
}

void ViewEvent::on_comboProjection_activated(const QString &/*arg1*/)
{
  bool visible = (ui->comboProjection->currentText() != "none");
  ui->plotProjection->setVisible(visible);
  if (visible)
    plot_current_event();
}

void ViewEvent::refresh_event()
{
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

  event_ = reader_->get_event(evt_idx);

  ui->eventX->display_record(event_.x());
  ui->eventY->display_record(event_.y());

  auto metrics = event_.metrics();
  if (ui->comboPlanes->currentText() == "X")
    metrics = event_.x().metrics();
  else if (ui->comboPlanes->currentText() == "Y")
    metrics = event_.y().metrics();

  QStringList list;
  for (auto a : metrics.data())
    list.push_back(QString::fromStdString(a.first));

  ui->searchBoxMetrics->setList(list);

  display_projection(event_);
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
  ui->plotProjection->clearAll();

  QPlot::Appearance profile;
  profile.default_pen = QPen(Qt::darkRed, 2);
  ui->plotProjection->addGraph(evt.get_projection(ui->comboProjection->currentText().toStdString()), profile);
  ui->plotProjection->setAxisLabels("position", "value");
  ui->plotProjection->setTitle(ui->comboProjection->currentText());
  ui->plotProjection->zoomOut();
}

bool ViewEvent::x_visible()
{
  return ui->comboPlanes->currentText().contains("X");
}

bool ViewEvent::y_visible()
{
  return ui->comboPlanes->currentText().contains("Y");
}

void ViewEvent::on_comboPlanes_currentIndexChanged(const QString&)
{
  ui->eventX->setVisible(ui->comboPlanes->currentText().contains("X"));
  ui->eventY->setVisible(ui->comboPlanes->currentText().contains("Y"));
  plot_current_event();
  display_params();
}

void ViewEvent::metrics_selected()
{
  NMX::MetricSet metrics;
  if (ui->comboPlanes->currentText() == "X")
    metrics = event_.x().metrics();
  else if (ui->comboPlanes->currentText() == "Y")
    metrics = event_.y().metrics();
  else
    metrics = event_.metrics();

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

void ViewEvent::on_push1x_clicked()
{
  makeCoordPopup(ui->push1x,
                 event_.x().metrics().with_prefix("strips_", false).with_suffix("_c", false));
}

void ViewEvent::on_push1y_clicked()
{
  makeCoordPopup(ui->push1y,
                 event_.x().metrics().with_prefix("timebins_", false).with_suffix("_c", false));
}

void ViewEvent::on_push2x_clicked()
{
  makeCoordPopup(ui->push2x,
                 event_.x().metrics().with_prefix("strips_", false).with_suffix("_c", false));
}

void ViewEvent::on_push2y_clicked()
{
  makeCoordPopup(ui->push2y,
                 event_.x().metrics().with_prefix("timebins_", false).with_suffix("_c", false));
}

void ViewEvent::makeCoordPopup(QPushButton* button,
                               NMX::MetricSet metric_set)
{
  QStringList list;
  list.push_back("none");
  for (auto &metric : metric_set.data())
    list.push_back(QString::fromStdString(metric.first));
  if (popupSearchDialog(button, list))
    set_point_metrics();
}

void ViewEvent::clicked_color1()
{
  pick_color(ui->color1);
  set_point_metrics();
}

void ViewEvent::clicked_color2()
{
  pick_color(ui->color2);
  set_point_metrics();
}

void ViewEvent::clicked_colorOverlay()
{
  pick_color(ui->colorOverlay);
  ui->eventX->set_overlay_color(ui->colorOverlay->color());
  ui->eventY->set_overlay_color(ui->colorOverlay->color());
}

void ViewEvent::pick_color(color_widgets::ColorPreview* prev)
{
  QDialog *popup = new QDialog(this);
  QBoxLayout *popupLayout = new QHBoxLayout(popup);
  popupLayout->setMargin(2);

  color_widgets::HueSlider* slider_
      = new color_widgets::HueSlider(Qt::Horizontal, popup);
  slider_->setColor(prev->color());

  popupLayout->addWidget(slider_);

  popup->move(prev->mapToGlobal(prev->rect().topLeft()));
  popup->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

  popup->exec();
  prev->setColor(slider_->color());
  delete popup;
}

