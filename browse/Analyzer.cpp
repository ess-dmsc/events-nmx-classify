#include <QSettings>
#include "Analyzer.h"
#include "ui_Analyzer.h"
#include "CustomLogger.h"


Analyzer::Analyzer(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::Analyzer)
  , histograms1d_(QVector<Histogram>())
  , boxes_model_(histograms1d_)
  , tests_model_(tests_.tests)
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

  ui->tableBoxes->setModel(&boxes_model_);
  ui->tableBoxes->setItemDelegate(&boxes_delegate_);
  ui->tableBoxes->horizontalHeader()->setStretchLastSection(true);
  ui->tableBoxes->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableBoxes->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableBoxes->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableBoxes->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->tableBoxes->show();

  connect(&boxes_model_, SIGNAL(data_changed()), this, SLOT(plot_boxes()));
  connect(&boxes_model_, SIGNAL(editing_finished()), this, SLOT(parameters_set()));
  connect(&boxes_delegate_, SIGNAL(edit_integer(QModelIndex,QVariant,int)),
          &boxes_model_, SLOT(setDataQuietly(QModelIndex,QVariant,int)));


  ui->tableTests->setModel(&tests_model_);
  ui->tableTests->setItemDelegate(&tests_delegate_);
  ui->tableTests->horizontalHeader()->setStretchLastSection(true);
  ui->tableTests->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableTests->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableTests->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableTests->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->tableTests->show();

  //  connect(&tests_model_, SIGNAL(data_changed()), this, SLOT(plot_boxes()));
  connect(&tests_model_, SIGNAL(editing_finished()), this, SLOT(rebuild_data()));
  connect(&tests_delegate_, SIGNAL(edit_integer(QModelIndex,QVariant,int)),
          &tests_model_, SLOT(setDataQuietly(QModelIndex,QVariant,int)));


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

  tests_model_.set_available_metrics(reader_->metrics());
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
    ui->tableTests->setEditTriggers(QAbstractItemView::AllEditTriggers);
  } else {
    ui->tableBoxes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableTests->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
}

void Analyzer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("weight_type_x", ui->comboWeightsX->currentText());
  settings.setValue("weight_type_y", ui->comboWeightsY->currentText());
  settings.setValue("weight_type_z", ui->comboWeightsZ->currentText());
}

void Analyzer::rebuild_data()
{
  if (!reader_|| !reader_->event_count())
    return;

  HistMap2D projection2d;
  HistMap1D histo;
  std::set<size_t> indices;

  auto weight_x = ui->comboWeightsX->currentText().toStdString();
  auto weight_y = ui->comboWeightsY->currentText().toStdString();
  auto weight_z = ui->comboWeightsZ->currentText().toStdString();

  auto xx = reader_->get_metric(weight_x);
  auto yy = reader_->get_metric(weight_y);
  auto zz = reader_->get_metric(weight_z);

  auto tests = tests_;
  MetricTest t;
  t.enabled = true;
  t.metric = weight_x;
  t.min = xx.min();
  t.max = xx.max();
  tests.tests.push_back(t);
  t.metric = weight_y;
  t.min = yy.min();
  t.max = yy.max();
  tests.tests.push_back(t);
  t.metric = weight_z;
  t.min = zz.min();
  t.max = zz.max();
  tests.tests.push_back(t);

  auto needed_metrics = tests.required_metrics();
  needed_metrics.push_back(weight_x);
  needed_metrics.push_back(weight_y);
  needed_metrics.push_back(weight_z);

  std::map<std::string, NMX::Metric> metrics;
  for (auto m : needed_metrics)
    metrics[m] = reader_->get_metric(m);

  ui->labelX->setText("   " + QString::fromStdString(xx.description()));
  ui->labelY->setText("   " + QString::fromStdString(yy.description()));
  ui->labelZ->setText("   " + QString::fromStdString(zz.description()));

  xx_norm = xx.normalizer();
  yy_norm = yy.normalizer();
  zz_norm = zz.normalizer();

  for (size_t eventID = 0; eventID < reader_->num_analyzed(); ++eventID)
  {
    if (!tests.validate(metrics, eventID))
      continue;

    projection2d[c2d(int32_t( (xx.data().at(eventID) - xx.min()) / xx_norm),
                     int32_t( (yy.data().at(eventID) - yy.min()) / yy_norm))] ++;

    histo[int( zz.data().at(eventID) / zz_norm) * zz_norm]++;

    indices.insert(eventID);
  }


  ui->plot2D->updatePlot((xx.max()-xx.min()) / xx_norm + 1,
                         (yy.max()-yy.min()) / yy_norm + 1, projection2d);
  ui->plot2D->setAxes(ui->comboWeightsX->currentText(), xx.min(), xx.max(),
                      ui->comboWeightsY->currentText(), yy.min(), yy.max(),
                      "Count");
  ui->plot2D->replot();

  ui->plotHistogram->clearAll();

  QPlot::Appearance profile;
  profile.default_pen = QPen(palette_[0], 2);

  ui->plotHistogram->addGraph(histo, profile);

  ui->plotHistogram->setAxisLabels(ui->comboWeightsZ->currentText(), "count");
  ui->plotHistogram->setTitle(ui->comboWeightsZ->currentText());
  ui->plotHistogram->zoomOut();

//  make_projections();
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
      boxes_model_.update();
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
      boxes_model_.update();
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
//  make_projections();
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
  boxes_model_.update();
  parameters_set();
}

void Analyzer::plot_block()
{
  QPlot::Appearance app;
  QColor cc (Qt::darkGray);
  cc.setAlpha(64);
  app.default_pen = QPen(cc, 2);

  //  ui->plotHistogram->setHighlight(QPlot::Marker1D(ui->spinMinZ->value(), app),
  //                                  QPlot::Marker1D(ui->spinMaxZ->value(), app));

  ui->plotHistogram->replotExtras();
  ui->plotHistogram->replot();
}

void Analyzer::set_metric_z(QString str)
{
  ui->comboWeightsZ->setCurrentText(str);
  rebuild_data();
}



void Analyzer::on_pushAddTest_clicked()
{
  tests_.tests.push_back(MetricTest());
  tests_model_.update();
}

void Analyzer::on_pushRemoveTest_clicked()
{
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  if (rows.size())
  {
    int row = rows.front().row();
    if ((row >= 0) && (row < histograms1d_.size()))
    {
      tests_.tests.remove(row);
      tests_model_.update();
    }
  }
}
