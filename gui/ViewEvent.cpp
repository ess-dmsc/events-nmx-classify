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

  ui->tableValues->setColumnCount(2);
  ui->tableValues->setHorizontalHeaderLabels({"parameter", "value"});
  ui->tableValues->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->tableValues->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableValues->setEditTriggers(QTableView::NoEditTriggers);
  ui->tableValues->horizontalHeader()->setStretchLastSection(true);
  ui->tableValues->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  NMX::Record rec;
  rec.analyze();
  for (auto &name : rec.point_categories())
  {
    ui->comboPlot->addItem(QString::fromStdString(name));
    ui->comboOverlay->addItem(QString::fromStdString(name));
  }
  ui->comboOverlay->addItem("none");
  ui->comboPlot->addItem("none");
  ui->comboPlot->addItem("everything");

  for (auto &name : rec.metrics().with_suffix("_c", false).with_prefix("strips", false).data())
  {
    ui->comboPoint1x->addItem(QString::fromStdString(name.first));
    ui->comboPoint2x->addItem(QString::fromStdString(name.first));
  }
  for (auto &name : rec.metrics().with_suffix("_c", false).with_prefix("timebins", false).data())
  {
    ui->comboPoint1y->addItem(QString::fromStdString(name.first));
    ui->comboPoint2y->addItem(QString::fromStdString(name.first));
  }
  ui->comboPoint1x->addItem("none");
  ui->comboPoint2x->addItem("none");
  ui->comboPoint1y->addItem("none");
  ui->comboPoint2y->addItem("none");

  NMX::Event evt;
  evt.analyze();
  for (auto &name : evt.projection_categories())
    ui->comboProjection->addItem(QString::fromStdString(name));
  ui->comboProjection->addItem("none");

  ui->eventX->set_title("X plane");
  ui->eventY->set_title("Y plane");

  loadSettings();

  ui->eventX->set_plot_type(ui->comboPlot->currentText());
  ui->eventY->set_plot_type(ui->comboPlot->currentText());

  ui->eventX->set_overlay_type(ui->comboOverlay->currentText());
  ui->eventY->set_overlay_type(ui->comboOverlay->currentText());

  ui->eventX->set_point_type1x(ui->comboPoint1x->currentText());
  ui->eventY->set_point_type1x(ui->comboPoint1x->currentText());
  ui->eventX->set_point_type1y(ui->comboPoint1y->currentText());
  ui->eventY->set_point_type1y(ui->comboPoint1y->currentText());

  ui->eventX->set_point_type2x(ui->comboPoint2x->currentText());
  ui->eventY->set_point_type2x(ui->comboPoint2x->currentText());
  ui->eventX->set_point_type2y(ui->comboPoint2y->currentText());
  ui->eventY->set_point_type2y(ui->comboPoint2y->currentText());

  ui->plotProjection->setScaleType("Linear");
  ui->plotProjection->setPlotStyle("Step center");
//  ui->plotProjection->set_visible_options(ShowOptions::style | ShowOptions::scale |
//                                          ShowOptions::labels | ShowOptions::thickness |
//                                          ShowOptions::grid | ShowOptions::save |
//                                          ShowOptions::zoom | ShowOptions::dither);

  on_comboPlanes_currentIndexChanged("");
  on_comboProjection_activated("");
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
  ui->comboPlot->setCurrentText(settings.value("plot", "everything").toString());
  ui->comboOverlay->setCurrentText(settings.value("overlay", "maxima").toString());
  ui->comboProjection->setCurrentText(settings.value("projection", "none").toString());
  ui->comboPoint1x->setCurrentText(settings.value("point1x").toString());
  ui->comboPoint2x->setCurrentText(settings.value("point2x").toString());
  ui->comboPoint1y->setCurrentText(settings.value("point1y").toString());
  ui->comboPoint2y->setCurrentText(settings.value("point2y").toString());
  ui->comboPlanes->setCurrentText(settings.value("show_planes", "X & Y").toString());
}

void ViewEvent::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("current_idx", ui->spinEventIdx->value());
  settings.setValue("plot", ui->comboPlot->currentText());
  settings.setValue("overlay", ui->comboOverlay->currentText());
  settings.setValue("projection", ui->comboProjection->currentText());
  settings.setValue("point1x", ui->comboPoint1x->currentText());
  settings.setValue("point2x", ui->comboPoint2x->currentText());
  settings.setValue("point1y", ui->comboPoint1y->currentText());
  settings.setValue("point2y", ui->comboPoint2y->currentText());
  settings.setValue("show_planes", ui->comboPlanes->currentText());
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
                            + "   [" + QString::number(evt_idx) + "]");
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


  event_ = reader_->get_event_with_metrics(evt_idx);

  if (evt_idx >= reader_->num_analyzed())
  {
    event_.set_parameters(params_);
    event_.analyze();
  }

  ui->eventX->display_record(event_.x());
  ui->eventY->display_record(event_.y());

  auto metrics = event_.metrics();
  if (ui->comboPlanes->currentText() == "X")
    metrics = event_.x().metrics();
  else if (ui->comboPlanes->currentText() == "Y")
    metrics = event_.y().metrics();

  ui->tableValues->clearContents();
  ui->tableValues->setRowCount(metrics.size());
  int i = 0;
  for (auto &a : metrics.data())
  {
    add_to_table(ui->tableValues, i, 0, a.first);
    add_to_table(ui->tableValues, i, 1, a.second.value.to_string());
    i++;
  }

  display_projection(event_);
  auto desc1x = event_.x().metrics().get(ui->comboPoint1x->currentText().toStdString()).description;
  ui->labelPoint1->setText(QString::fromStdString(desc1x));
  auto desc2x = event_.x().metrics().get(ui->comboPoint2x->currentText().toStdString()).description;
  ui->labelPoint2->setText(QString::fromStdString(desc2x));
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


void ViewEvent::on_comboPlanes_currentIndexChanged(const QString&)
{
  ui->eventX->setVisible(ui->comboPlanes->currentText().contains("X"));
  ui->eventY->setVisible(ui->comboPlanes->currentText().contains("Y"));
  plot_current_event();
}

void ViewEvent::on_comboPoint1x_currentIndexChanged(const QString &arg1)
{
  ui->eventX->set_point_type1x(ui->comboPoint1x->currentText());
  ui->eventY->set_point_type1x(ui->comboPoint1x->currentText());
  plot_current_event();
}

void ViewEvent::on_comboPoint2x_currentIndexChanged(const QString &arg1)
{
  ui->eventX->set_point_type2x(ui->comboPoint2x->currentText());
  ui->eventY->set_point_type2x(ui->comboPoint2x->currentText());
  plot_current_event();
}

void ViewEvent::on_comboPoint1y_currentIndexChanged(const QString &arg1)
{
  ui->eventX->set_point_type1y(ui->comboPoint1y->currentText());
  ui->eventY->set_point_type1y(ui->comboPoint1y->currentText());
  plot_current_event();
}

void ViewEvent::on_comboPoint2y_currentIndexChanged(const QString &arg1)
{
  ui->eventX->set_point_type2y(ui->comboPoint2y->currentText());
  ui->eventY->set_point_type2y(ui->comboPoint2y->currentText());
  plot_current_event();
}
