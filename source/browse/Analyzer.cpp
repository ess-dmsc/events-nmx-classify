#include <QSettings>
#include "Analyzer.h"
#include "ui_Analyzer.h"
#include "CustomLogger.h"
#include "doFit.h"

Analyzer::Analyzer(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::Analyzer)
{
  ui->setupUi(this);
  ui->comboFit->addItem("none");
  ui->comboFit->addItem("left");
  ui->comboFit->addItem("right");
  ui->comboFit->addItem("double");

  ui->plotHistogram->setScaleType("Linear");
  ui->plotHistogram->setPlotStyle("Step center");

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

  connect(ui->tableTests->selectionModel(),
          SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this, SLOT(filterSelectionChanged()));

  connect(&tests_model_, SIGNAL(editing_finished()), this, SLOT(rebuildFilteredList()));
  connect(&tests_delegate_, SIGNAL(edit_integer(QModelIndex,QVariant,int)),
          &tests_model_, SLOT(setDataQuietly(QModelIndex,QVariant,int)));

  connect(ui->pushX, SIGNAL(selectionChanged(QString)), this, SLOT(replot()));
  connect(ui->pushY, SIGNAL(selectionChanged(QString)), this, SLOT(replot()));
  connect(ui->pushMetric1D, SIGNAL(selectionChanged(QString)), this, SLOT(replot()));

  for (auto s : histogram1d_.values())
    ui->comboAverage->addItem(QString::fromStdString(s));

  loadSettings();
}

Analyzer::~Analyzer()
{
  saveSettings();
  delete ui;
}

void Analyzer::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.beginGroup("DatasetMetrics");
  ui->pushX->setText(settings.value("weight_type_x", "none").toString());
  ui->pushY->setText(settings.value("weight_type_y", "none").toString());
  ui->pushMetric1D->setText(settings.value("metric_1d", "none").toString());
  ui->comboFit->setCurrentText(settings.value("fit").toString());
  ui->doubleUnits->setValue(settings.value("units", 400).toDouble());
}

void Analyzer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.beginGroup("DatasetMetrics");
  settings.setValue("weight_type_x", ui->pushX->text());
  settings.setValue("weight_type_y", ui->pushY->text());
  settings.setValue("metric_1d", ui->pushMetric1D->text());
  settings.setValue("fit", ui->comboFit->currentText());
  settings.setValue("units", ui->doubleUnits->value());
}

void Analyzer::set_new_source(std::shared_ptr<NMX::File> r)
{
  reader_ = r;
  rebuildFilteredList();
  populate_combos();
}

void Analyzer::populate_combos()
{
  if (!reader_)
    return;

  auto metrics = reader_->metrics();
  tests_model_.set_available_metrics(metrics);
  auto list = getMetricsList(metrics);
  ui->pushX->setList(list);
  ui->pushY->setList(list);
  ui->pushMetric1D->setList(list);
}

void Analyzer::rebuildFilteredList()
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

  double percent = double(indices_.size()) / double(reader_->event_count()) * 100.0;
  ui->labelFilterResults->setText(
        "   Filtered events: " + QString::number(percent) +
        "% (" + QString::number(indices_.size()) + "/" +
               QString::number(reader_->event_count()) + ")");
  replot();
  emit select_indices(indices_);
}

void Analyzer::replot()
{
  if (!reader_|| !reader_->event_count())
    return;

  HistMap2D projection2d;
  histogram1d_.clear();

  auto xx = reader_->get_metric(ui->pushX->text().toStdString());
  auto yy = reader_->get_metric(ui->pushY->text().toStdString());
  auto zz = reader_->get_metric(ui->pushMetric1D->text().toStdString());

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

      histogram1d_.add_one(int( zz.data().at(eventID) / zz_norm) * zz_norm);
    }

  ui->plot2D->updatePlot((xx.max()-xx.min()) / xx_norm + 1,
                         (yy.max()-yy.min()) / yy_norm + 1, projection2d);
  ui->plot2D->setAxes(ui->pushX->text(), xx.min(), xx.max(),
                      ui->pushY->text(), yy.min(), yy.max(),
                      "Count");
  ui->plot2D->replot();

  replot1d();
}

void Analyzer::replot1d()
{
  double average = histogram1d_.get_value(ui->comboAverage->currentText().toStdString());

  ui->labelAverage->setText(" = " + QString::number(average));

  auto fit_type = ui->comboFit->currentText().toStdString();

  EdgeFitter fitter(histogram1d_.map());
  fitter.analyze(fit_type);

  if (fit_type != "none")
    ui->labelFit->setText("   " + QString::fromStdString(fitter.info(ui->doubleUnits->value())));
  else
    ui->labelFit->setText("   No edge fit model selected");

  ui->plotHistogram->clearAll();
  //  QPlot::Appearance profile;
  //  profile.default_pen = QPen(palette_[0], 2);
  ui->plotHistogram->addGraph(histogram1d_.map(), QPen(palette_[0], 2));
  ui->plotHistogram->addGraph(fitter.get_fit_hist(4), QPen(palette_[1], 2));
  QPlot::Marker1D marker(average);
  ui->plotHistogram->setHighlight(marker, marker);
  ui->plotHistogram->setAxisLabels(ui->pushMetric1D->text(), "count");
  ui->plotHistogram->replotExtras();
  ui->plotHistogram->zoomOut();
}

void Analyzer::set_metric_z(QString str)
{
  ui->pushMetric1D->setText(str);
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

QStringList Analyzer::getMetricsList(std::list<std::string> metric_set)
{
  QStringList list;
  list.push_back("none");
  for (auto &name : metric_set)
    list.push_back(QString::fromStdString(name));
  return list;
}

void Analyzer::on_pushFilterFromPlot_clicked()
{
  auto name = ui->pushMetric1D->text().toStdString();
  auto filter = tests_model_.tests();
  for (auto f : filter.tests)
    if (f.metric == name)
      return;
  MetricTest newtest;
  newtest.metric = name;
  if (reader_)
  {
    auto metric_info = reader_->get_metric(name);
    newtest.min = metric_info.min();
    newtest.max = metric_info.max();
  }
  filter.tests.push_back(newtest);
  tests_model_.set_tests(filter);
}

void Analyzer::on_pushFilterToPlot_clicked()
{
  auto filter = tests_model_.tests();
  int row = -1;
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  if (rows.size())
    row = rows.front().row();

  if ((row < 0) && (row >= filter.tests.size()))
    return;

  auto name = tests_model_.tests().tests.at(row).metric;
  ui->pushMetric1D->setText(QString::fromStdString(name));
  replot();
}

void Analyzer::filterSelectionChanged()
{
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  ui->pushFilterToPlot->setEnabled(rows.size());
  ui->pushRemoveTest->setEnabled(rows.size());
}


HistMap1D Histogram1D::map() const
{
  return map_;
}

HistList1D Histogram1D::list() const
{
  return to_list(map_);
}

void Histogram1D::add_one(double bin)
{
  map_[bin]++;
  count_++;
}

void Histogram1D::clear()
{
  map_.clear();
  count_ = 0;
}

std::list<std::string> Histogram1D::values()
{
  return {"mean", "median", "mode", "midrange", "RMS", "harmonic mean"};
}

double Histogram1D::get_value(std::string t) const
{
  if (t == "mean")
    return mean();
  else if (t == "median")
    return median();
  else if (t == "mode")
    return mode();
  else if (t == "midrange")
    return midrange();
  else if (t == "RMS")
    return RMS();
  else if (t == "harmonic mean")
    return harmonic_mean();
  return std::numeric_limits<double>::quiet_NaN();
}

double Histogram1D::mean() const
{
  double sum {0};
  for (const auto& m : map_)
    sum += m.first * m.second;
  return sum / static_cast<double>(count_);
}

double Histogram1D::median() const
{
  double med_count = static_cast<double>(count_) / 2.0;
  double mcount {0};
  double prev = std::numeric_limits<double>::quiet_NaN();
  for (const auto& m : map_)
  {
    mcount += m.second;
    if (mcount > med_count)
      break;
    prev = m.first;
  }
  return prev;
}

double Histogram1D::mode() const
{
  double maxcount = std::numeric_limits<double>::min();
  double maxbin = std::numeric_limits<double>::quiet_NaN();
  for (const auto& m : map_)
    if (m.second > maxcount)
    {
      maxcount = m.second;
      maxbin = m.first;
    }
  return maxbin;
}

double Histogram1D::harmonic_mean() const
{
  double sum {0};
  for (const auto& m : map_)
    sum += m.second / m.first;
  return static_cast<double>(count_) / sum;
}

double Histogram1D::RMS() const
{
  double sum {0};
  for (const auto& m : map_)
    sum += m.first * m.first * m.second;
  return sqrt(sum / static_cast<double>(count_));
}

double Histogram1D::midrange() const
{
  if (map_.empty())
    return std::numeric_limits<double>::quiet_NaN();
  return (map_.begin()->first + map_.rbegin()->first) / 2.0;
}

void Analyzer::on_comboAverage_currentIndexChanged(int index)
{
  replot1d();
}
