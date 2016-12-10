#include <QSettings>
#include "Analyzer.h"
#include "ui_Analyzer.h"
#include "CustomLogger.h"
#include "doFit.h"

Analyzer::Analyzer(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::Analyzer)
//  , tests_model_(tests_.tests)
{
  ui->setupUi(this);

  ui->plotHistogram->setScaleType("Linear");
  ui->plotHistogram->setPlotStyle("Step center");
  //  ui->plotHistogram->set_visible_options(ShowOptions::zoom | ShowOptions::thickness | ShowOptions::scale | ShowOptions::grid | ShowOptions::save);

  ui->plot2D->setAntialiased(false);
  ui->plot2D->setScaleType("Linear");
  ui->plot2D->setGradient("YlGnBu5");
  ui->plot2D->setShowGradientLegend(true);

  ui->tableTests->setModel(&tests_model_);
  ui->tableTests->setItemDelegate(&tests_delegate_);
  ui->tableTests->horizontalHeader()->setStretchLastSection(true);
  ui->tableTests->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableTests->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableTests->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableTests->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->tableTests->show();

  connect(&tests_model_, SIGNAL(editing_finished()), this, SLOT(rebuild()));
  connect(&tests_delegate_, SIGNAL(edit_integer(QModelIndex,QVariant,int)),
          &tests_model_, SLOT(setDataQuietly(QModelIndex,QVariant,int)));


  ui->comboFit->addItem("none");
  ui->comboFit->addItem("left");
  ui->comboFit->addItem("right");
  ui->comboFit->addItem("double");

  loadSettings();
}

void Analyzer::populate_combos()
{
  ui->comboWeightsX->blockSignals(true);
  ui->comboWeightsY->blockSignals(true);
  ui->comboWeightsZ->blockSignals(true);

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

  ui->comboWeightsX->blockSignals(false);
  ui->comboWeightsY->blockSignals(false);
  ui->comboWeightsZ->blockSignals(false);
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
    ui->tableTests->setEditTriggers(QAbstractItemView::AllEditTriggers);
  } else {
    ui->tableTests->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }
}

void Analyzer::set_new_source(std::shared_ptr<NMX::FileAPV> r)
{
  reader_ = r;

  if (ui->comboWeightsX->count())
    saveSettings();

  rebuild();

  populate_combos();

  loadSettings();

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
  ui->comboFit->setCurrentText(settings.value("fit").toString());
  ui->doubleUnits->setValue(settings.value("units", 400).toDouble());
}

void Analyzer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("weight_type_x", ui->comboWeightsX->currentText());
  settings.setValue("weight_type_y", ui->comboWeightsY->currentText());
  settings.setValue("weight_type_z", ui->comboWeightsZ->currentText());
  settings.setValue("fit", ui->comboFit->currentText());
  settings.setValue("units", ui->doubleUnits->value());
}

void Analyzer::rebuild()
{
  if (!reader_|| !reader_->event_count())
    return;

  indices_.clear();

  auto tests = tests_model_.tests();

  std::map<std::string, NMX::Metric> metrics;
  for (auto m : tests.required_metrics())
    metrics[m] = reader_->get_metric(m);

  for (size_t eventID = 0; eventID < reader_->num_analyzed(); ++eventID)
    if (tests.validate(metrics, eventID))
      indices_.insert(eventID);

  replot();
  emit select_indices(indices_);
}

void Analyzer::replot()
{
  if (!reader_|| !reader_->event_count())
    return;

  HistMap2D projection2d;
  histogram1d_.clear();

  auto xx = reader_->get_metric(ui->comboWeightsX->currentText().toStdString());
  auto yy = reader_->get_metric(ui->comboWeightsY->currentText().toStdString());
  auto zz = reader_->get_metric(ui->comboWeightsZ->currentText().toStdString());

  ui->labelX->setText("   " + QString::fromStdString(xx.description()));
  ui->labelY->setText("   " + QString::fromStdString(yy.description()));
  ui->labelZ->setText("   " + QString::fromStdString(zz.description()));

  auto xx_norm = xx.normalizer();
  auto yy_norm = yy.normalizer();
  auto zz_norm = zz.normalizer();

  if ((xx.data().size() > 0) &&
      (xx.data().size() == yy.data().size()) &&
      (xx.data().size() == zz.data().size()))
    for (auto eventID : indices_)
    {
      projection2d[c2d(int32_t( (xx.data().at(eventID) - xx.min()) / xx_norm),
                       int32_t( (yy.data().at(eventID) - yy.min()) / yy_norm))] ++;

      histogram1d_[int( zz.data().at(eventID) / zz_norm) * zz_norm]++;
    }

  ui->plot2D->updatePlot((xx.max()-xx.min()) / xx_norm + 1,
                         (yy.max()-yy.min()) / yy_norm + 1, projection2d);
  ui->plot2D->setAxes(ui->comboWeightsX->currentText(), xx.min(), xx.max(),
                      ui->comboWeightsY->currentText(), yy.min(), yy.max(),
                      "Count");
  ui->plot2D->replot();

  replot1d();
}

void Analyzer::replot1d()
{
  ui->plotHistogram->clearAll();

  QPlot::Appearance profile;

  profile.default_pen = QPen(palette_[0], 2);
  ui->plotHistogram->addGraph(histogram1d_, profile);

  auto fit_type = ui->comboFit->currentText().toStdString();

  EdgeFitter fitter(histogram1d_);
  fitter.analyze(fit_type);

  if (fit_type != "none")
    ui->labelFit->setText("   " + QString::fromStdString(fitter.info(ui->doubleUnits->value())));
  else
    ui->labelFit->setText("   No edge fit model selected");

  profile.default_pen = QPen(palette_[1], 2);
  ui->plotHistogram->addGraph(fitter.get_fit_hist(4), profile);

  ui->plotHistogram->setAxisLabels(ui->comboWeightsZ->currentText(), "count");
  ui->plotHistogram->zoomOut();
}

void Analyzer::plot_boxes()
{
  std::list<QPlot::MarkerBox2D> boxes;

//  for (int i=0; i < histograms1d_.size(); ++i)
//  {
//    const auto &p = histograms1d_[i];

//    QPlot::MarkerBox2D box;
//    box.x1 = p.x1();
//    box.x2 = p.x2();
//    box.y1 = p.y1();
//    box.y2 = p.y2();
//    box.selectable = false;
//    //    box.selected = true;
//    box.fill = box.border = p.color;
//    box.fill.setAlpha(box.fill.alpha() * 0.15);
//    box.label = QString::number(i);

//    boxes.push_back(box);
//  }

  ui->plot2D->setBoxes(boxes);
  ui->plot2D->replotExtras();
}

void Analyzer::on_comboWeightsX_currentIndexChanged(const QString& /*arg1*/)
{
  replot();
}

void Analyzer::on_comboWeightsY_currentIndexChanged(const QString& /*arg1*/)
{
  replot();
}

void Analyzer::on_comboWeightsZ_currentIndexChanged(const QString& /*arg1*/)
{
  replot();
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
  replot();
}



void Analyzer::on_pushAddTest_clicked()
{
  auto filter = tests_model_.tests();
  filter.tests.push_back(MetricTest());
  tests_model_.set_tests(filter);
}

void Analyzer::on_pushRemoveTest_clicked()
{
  auto filter = tests_model_.tests();
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  if (rows.size())
  {
    int row = rows.front().row();
    if ((row >= 0) && (row < filter.tests.size()))
    {
      filter.tests.remove(row);
    }
  }
  tests_model_.set_tests(filter);
}

void Analyzer::on_comboFit_currentTextChanged(const QString &arg1)
{
  replot1d();
}

void Analyzer::on_doubleUnits_editingFinished()
{
  replot1d();
}
