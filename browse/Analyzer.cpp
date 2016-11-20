#include <QSettings>
#include "Analyzer.h"
#include "ui_Analyzer.h"
#include "CustomLogger.h"


Analyzer::Analyzer(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::Analyzer)
  , histograms1d_(QVector<Histogram>())
  , model_(histograms1d_)
{
  ui->setupUi(this);

  Histogram params;
  params.visible = true;
  params.color = QColor(0,0,0,0);
  params.set_x(0, 256);
  params.set_y(0, 256);
  histograms1d_.push_back(params);

  ui->plotHistogram->setScaleType("Linear");
  ui->plotHistogram->setPlotStyle("Step center");
//  ui->plotHistogram->set_visible_options(ShowOptions::zoom | ShowOptions::thickness | ShowOptions::scale | ShowOptions::grid | ShowOptions::save);

  ui->plot2D->setAntialiased(false);
  ui->plot2D->setScaleType("Linear");
  ui->plot2D->setGradient("YlGnBu5");
  ui->plot2D->setShowGradientLegend(true);
  connect(ui->plot2D, SIGNAL(clickedPlot(double,double,Qt::MouseButton)), this, SLOT(update_box(double, double, Qt::MouseButton)));

  ui->tableBoxes->setModel(&model_);
  ui->tableBoxes->setItemDelegate(&delegate_);
  ui->tableBoxes->horizontalHeader()->setStretchLastSection(true);
  ui->tableBoxes->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableBoxes->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableBoxes->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableBoxes->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->tableBoxes->show();

  connect(&model_, SIGNAL(data_changed()), this, SLOT(plot_boxes()));
  connect(&model_, SIGNAL(editing_finished()), this, SLOT(parameters_set()));
  connect(&delegate_, SIGNAL(edit_integer(QModelIndex,QVariant,int)),
          &model_, SLOT(setDataQuietly(QModelIndex,QVariant,int)));

  loadSettings();
}

void Analyzer::populate_combos()
{
  ui->comboWeightsX->clear();
  ui->comboWeightsY->clear();
  ui->comboWeightsZ->clear();

  if (!reader_)
    return;

  ui->comboWeightsZ->addItem("none");

  for (auto &c : reader_->metrics())
  {
    ui->comboWeightsX->addItem(QString::fromStdString(c));
    ui->comboWeightsY->addItem(QString::fromStdString(c));
    ui->comboWeightsZ->addItem(QString::fromStdString(c));
  }
}

Analyzer::~Analyzer()
{
  saveSettings();
  delete ui;
}

void Analyzer::enableIO(bool enable)
{
  bool en = reader_ && reader_->event_count() && enable;
  ui->comboWeightsX->setEnabled(en);
  ui->comboWeightsY->setEnabled(en);
  ui->comboWeightsZ->setEnabled(en);

  if (enable) {
    ui->tableBoxes->setEditTriggers(QAbstractItemView::AllEditTriggers);
  } else {
    ui->tableBoxes->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }
}

void Analyzer::set_new_source(std::shared_ptr<NMX::FileAPV> r)
{
  reader_ = r;

  if (ui->comboWeightsX->count())
    saveSettings();

  populate_combos();

  loadSettings();

  rebuild_data();
  plot_block();
  plot_boxes();
}

void Analyzer::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  ui->comboWeightsX->setCurrentText(settings.value("weight_type_x", "X_entry_strip").toString());
  ui->comboWeightsY->setCurrentText(settings.value("weight_type_y", "Y_entry_strip").toString());
  ui->comboWeightsZ->setCurrentText(settings.value("weight_type_z").toString());

  ui->spinMinX->setValue(settings.value("min_x", 0).toInt());
  ui->spinMaxX->setValue(settings.value("max_x", 1000000).toInt());

  ui->spinMinY->setValue(settings.value("min_y", 0).toInt());
  ui->spinMaxY->setValue(settings.value("max_y", 1000000).toInt());

  ui->spinMinZ->setValue(settings.value("min_z", 0).toInt());
  ui->spinMaxZ->setValue(settings.value("max_z", 1000000).toInt());
}

void Analyzer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("weight_type_x", ui->comboWeightsX->currentText());
  settings.setValue("weight_type_y", ui->comboWeightsY->currentText());
  settings.setValue("weight_type_z", ui->comboWeightsZ->currentText());

  settings.setValue("min_x", ui->spinMinX->value());
  settings.setValue("max_x", ui->spinMaxX->value());

  settings.setValue("min_y", ui->spinMinY->value());
  settings.setValue("max_y", ui->spinMaxY->value());

  settings.setValue("min_z", ui->spinMinZ->value());
  settings.setValue("max_z", ui->spinMaxZ->value());
}

void Analyzer::rebuild_data()
{
  if (!reader_|| !reader_->event_count())
    return;

  data_.clear();

  auto weight_x = ui->comboWeightsX->currentText().toStdString();
  auto weight_y = ui->comboWeightsY->currentText().toStdString();
  auto weight_z = ui->comboWeightsZ->currentText().toStdString();

  auto xx = reader_->get_metric(weight_x);
  auto yy = reader_->get_metric(weight_y);
  auto zz = reader_->get_metric(weight_z);

  ui->labelX->setText("   " + QString::fromStdString(xx.description()));
  ui->labelY->setText("   " + QString::fromStdString(yy.description()));
  ui->labelZ->setText("   " + QString::fromStdString(zz.description()));

  if (xx.data().size() != yy.data().size())
  {
    make_projections();
    return;
  }

  xx_norm = xx.normalizer();
  yy_norm = yy.normalizer();
  zz_norm = zz.normalizer();

  for (size_t eventID = 0; eventID < reader_->num_analyzed(); ++eventID)
  {
    if ((eventID >= xx.data().size()) || (eventID >= yy.data().size()) || (eventID >= zz.data().size()))
      continue;

    data_[int(zz.data().at(eventID) / zz_norm)]
         [std::pair<int,int>({int(xx.data().at(eventID) / xx_norm),
                              int(yy.data().at(eventID) / yy_norm)})].push_back(eventID);
  }

  for (auto &i : histograms1d_)
    i.reset_data();

  make_projections();
}

void Analyzer::make_projections()
{
  std::set<size_t> indices;

  int min_x = ui->spinMinX->value();
  int max_x = ui->spinMaxX->value();

  int min_y = ui->spinMinY->value();
  int max_y = ui->spinMaxY->value();

  int min_z = ui->spinMinZ->value();
  int max_z = ui->spinMaxZ->value();

  HistMap2D projection2d;

  int32_t xmin{std::numeric_limits<int32_t>::max()};
  int32_t xmax{std::numeric_limits<int32_t>::min()};
  int32_t ymin{std::numeric_limits<int32_t>::max()};
  int32_t ymax{std::numeric_limits<int32_t>::min()};

  for (auto &ms : data_)
  {
    auto &z = ms.first;
    for (auto &mi : ms.second)
    {
      int x = mi.first.first;
      int y = mi.first.second;

      if ((min_x <= x) && (x <= max_x) &&
          (min_y <= y) && (y <= max_y) &&
          (min_z <= z) && (z <= max_z))
      {
        projection2d[{mi.first.first, mi.first.second}] += mi.second.size();
        std::copy( mi.second.begin(), mi.second.end(), std::inserter( indices, indices.end() ) );

        xmin = std::min(xmin, x);
        xmax = std::max(xmax, x);

        ymin = std::min(ymin, y);
        ymax = std::max(ymax, y);
      }

      for (auto &i : histograms1d_)
        i.add_to_hist(x, y, z, mi.second.size());
    }
  }

//  DBG << "x " << xmin << " - " << xmax;
//  DBG << "y " << ymin << " - " << ymax;

  for (auto &i : histograms1d_)
    i.close_data();

  ui->plot2D->updatePlot(xmax-xmin+1, ymax-ymin+1, projection2d);
  ui->plot2D->setAxes(ui->comboWeightsX->currentText(), xmin * xx_norm, xmax * xx_norm,
                      ui->comboWeightsY->currentText(), ymin * yy_norm, ymax * yy_norm,
                      "Count");
  ui->plot2D->replot();

  update_histograms();

  emit select_indices(indices);
}

void Analyzer::update_histograms()
{
  ui->plotHistogram->clearAll();

  for (int i=0; i < histograms1d_.size(); ++i)
  {
    if (!histograms1d_[i].visible)
      continue;

    HistMap1D histo;
    for (auto &b : histograms1d_[i].data())
      histo[b.first] = b.second;

    QPlot::Appearance profile;
    profile.default_pen = QPen(palette_[i % palette_.size()], 2);

    ui->plotHistogram->addGraph(histo, profile);
  }

  ui->plotHistogram->setAxisLabels(ui->comboWeightsZ->currentText(), "count");
  ui->plotHistogram->setTitle(ui->comboWeightsZ->currentText()
                              + "  (normalized by: " + QString::number(zz_norm) + ")");
  ui->plotHistogram->zoomOut();

  plot_block();
}

void Analyzer::update_box(double x, double y, Qt::MouseButton button)
{
  auto rows = ui->tableBoxes->selectionModel()->selectedRows();
  if (rows.size())
  {
    int row = rows.front().row();
//    DBG << "change for row " << row << " " << x << " " << y;
    if ((row >= 0) && (row < histograms1d_.size()))
    {
      if (button == Qt::LeftButton)
      {
        histograms1d_[row].set_center_x(static_cast<int64_t>(x));
        histograms1d_[row].set_center_y(static_cast<int64_t>(y));
      }
      histograms1d_[row].visible = (button == Qt::LeftButton);
      plot_boxes();
      model_.update();
      parameters_set();
    }
  }
}

void Analyzer::on_pushRemoveBox_clicked()
{
  auto rows = ui->tableBoxes->selectionModel()->selectedRows();
  if (rows.size())
  {
    int row = rows.front().row();
    if ((row >= 0) && (row < histograms1d_.size()))
    {
      histograms1d_.remove(row);
      plot_boxes();
      model_.update();
      parameters_set();
    }
  }
}

void Analyzer::plot_boxes()
{
  std::list<QPlot::MarkerBox2D> boxes;

  for (int i=0; i < histograms1d_.size(); ++i)
  {
    const auto &p = histograms1d_[i];
    if (!p.visible)
      continue;

    QPlot::MarkerBox2D box;
    box.x1 = p.x1();
    box.x2 = p.x2();
    box.y1 = p.y1();
    box.y2 = p.y2();
    box.selectable = false;
//    box.selected = true;
    box.fill = box.border = p.color;
    box.fill.setAlpha(box.fill.alpha() * 0.15);
    box.label = QString::number(i);

    boxes.push_back(box);
  }

  ui->plot2D->setBoxes(boxes);
  ui->plot2D->replotExtras();
}

void Analyzer::parameters_set()
{
  make_projections();
}

void Analyzer::on_comboWeightsX_currentIndexChanged(const QString& /*arg1*/)
{
  rebuild_data();
}

void Analyzer::on_comboWeightsY_currentIndexChanged(const QString& /*arg1*/)
{
  rebuild_data();
}

void Analyzer::on_comboWeightsZ_currentIndexChanged(const QString& /*arg1*/)
{
  rebuild_data();
}

void Analyzer::on_pushAddBox_clicked()
{
  Histogram params;

  params.visible = true;
  params.color = palette_[histograms1d_.size() % palette_.size()];
  params.set_x(0, 250);
  params.set_y(0, 250);

  histograms1d_.push_back(params);

  plot_boxes();
  model_.update();
  parameters_set();
}

void Analyzer::on_spinMinX_editingFinished()
{
  make_projections();
}

void Analyzer::on_spinMaxX_editingFinished()
{
  make_projections();
}

void Analyzer::on_spinMinY_editingFinished()
{
  make_projections();
}

void Analyzer::on_spinMaxY_editingFinished()
{
  make_projections();
}

void Analyzer::on_spinMinZ_editingFinished()
{
  make_projections();
}

void Analyzer::on_spinMaxZ_editingFinished()
{
  make_projections();
}

void Analyzer::on_spinMinZ_valueChanged(int /*arg1*/)
{
  plot_block();
}

void Analyzer::on_spinMaxZ_valueChanged(int /*arg1*/)
{
  plot_block();
}

void Analyzer::plot_block()
{
  QPlot::Appearance app;
  QColor cc (Qt::darkGray);
  cc.setAlpha(64);
  app.default_pen = QPen(cc, 2);

  ui->plotHistogram->setHighlight(QPlot::Marker1D(ui->spinMinZ->value(), app),
                                  QPlot::Marker1D(ui->spinMaxZ->value(), app));

  ui->plotHistogram->replotExtras();
  ui->plotHistogram->replot();
}

void Analyzer::set_metric_z(QString str)
{
  ui->comboWeightsZ->setCurrentText(str);
  rebuild_data();
}


