#include <QSettings>
#include "Analyzer.h"
#include "ui_Analyzer.h"
#include "CustomLogger.h"
#include "doFit.h"
#include "qt_util.h"
#include "histogram_h5.h"

#include "DialogVary.h"
#include "JsonH5.h"

Analyzer::Analyzer(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::Analyzer)
{
  ui->setupUi(this);
  ui->pushVary->setEnabled(false);
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
  ui->comboAverage->addItem("fitted edge");

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

  MetricFilter filter;
  load(filter, settings, "Filters");
  tests_model_.set_tests(filter);
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

  save(tests_model_.tests(), settings, "Filters");
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

  histogram1d_.clear();
  histogram2d_.clear();

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
      histogram2d_[QPlot::c2d(int32_t( (xx.data().at(eventID) - xx.min()) / xx_norm),
                              int32_t( (yy.data().at(eventID) - yy.min()) / yy_norm))] ++;

      histogram1d_.add_one(int( zz.data().at(eventID) / zz_norm) * zz_norm);
    }

  ui->plot2D->updatePlot((xx.max()-xx.min()) / xx_norm + 1,
                         (yy.max()-yy.min()) / yy_norm + 1, histogram2d_);
  ui->plot2D->setAxes(ui->pushX->text(), xx.min(), xx.max(),
                      ui->pushY->text(), yy.min(), yy.max(),
                      "Count");
  ui->plot2D->replot();

  replot1d();
}

void Analyzer::replot1d()
{
  auto average_type = ui->comboAverage->currentText().toStdString();
  double average = 0;


  auto fit_type = ui->comboFit->currentText().toStdString();

  EdgeFitter fitter(histogram1d_.map());
  fitter.analyze(fit_type);

  if (average_type == "fitted edge")
    average = fitter.position(1);
  else
    average= histogram1d_.get_value(ui->comboAverage->currentText().toStdString());

  if (fit_type != "none")
    ui->labelFit->setText(QString::fromStdString(fitter.info(ui->doubleUnits->value())));
  else
    ui->labelFit->setText("   No edge fit model selected");

  DBG << fitter.info(ui->doubleUnits->value());

  ui->plotHistogram->clearAll();
  //  QPlot::Appearance profile;
  //  profile.default_pen = QPen(palette_[0], 2);
  ui->plotHistogram->addGraph(histogram1d_.map(), QPen(palette_[0], 2));
  ui->plotHistogram->addGraph(fitter.get_fit_hist(10), QPen(palette_[2], 2));
  QPlot::Marker1D marker(average);
  ui->labelAverage->setText(" = " + QString::number(average));
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
  filter.add(MetricTest());
  tests_model_.set_tests(filter);
}

void Analyzer::on_pushRemoveTest_clicked()
{
  auto filter = tests_model_.tests();
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  if (rows.size())
  {
    int row = rows.front().row();
    if (row >= 0)
      filter.remove(row);
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
  //  for (auto f : filter.tests)
  //    if (f.metric == name)
  //      return;
  MetricTest newtest;
  newtest.metric = name;
  if (reader_)
  {
    auto metric_info = reader_->get_metric(name);
    newtest.min = metric_info.min();
    newtest.max = metric_info.max();
  }
  filter.add(newtest);
  tests_model_.set_tests(filter);
}

void Analyzer::on_pushUpTest_clicked()
{
  auto filter = tests_model_.tests();
  int row = -1;
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  if (rows.size())
    row = rows.front().row();

  if ((row < 1) && (row >= int(filter.size())))
    return;

  filter.up(row);
  tests_model_.set_tests(filter);
  ui->tableTests->selectRow(row-1);
}

void Analyzer::on_pushDownTest_clicked()
{
  auto filter = tests_model_.tests();
  int row = -1;
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  if (rows.size())
    row = rows.front().row();

  if ((row < 0) && ((row+1) >= int(filter.size())))
    return;

  filter.down(row);
  tests_model_.set_tests(filter);
  ui->tableTests->selectRow(row+1);
}

void Analyzer::on_pushFilterToPlot_clicked()
{
  auto filter = tests_model_.tests();
  int row = -1;
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  if (rows.size())
    row = rows.front().row();

  if ((row < 0) && (row >= int(filter.size())))
    return;

  auto name = tests_model_.tests().test(row).metric;
  ui->pushMetric1D->setText(QString::fromStdString(name));
  replot();
}

void Analyzer::filterSelectionChanged()
{
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  ui->pushRemoveTest->setEnabled(rows.size());
  ui->pushFilterToPlot->setEnabled(rows.size() == 1);
  ui->pushVary->setEnabled(rows.size() == 1);
}



void Analyzer::on_comboAverage_currentIndexChanged(int index)
{
  replot1d();
}

void Analyzer::on_pushSave1D_clicked()
{
  QSettings settings;
  settings.beginGroup("Program");
  auto data_directory = settings.value("data_directory", "").toString();
  QString fileName = CustomSaveFileDialog(this, "Save project",
                                          data_directory, "hdf5 (*.h5)");
  if (fileName.isEmpty())
    return;

  bool ok;
  QString text = QInputDialog::getText(this, "Name histogram",
                                       "Histogram name:", QLineEdit::Normal,
                                       "", &ok);
  if (!ok || text.isEmpty())
    return;

  try
  {
    H5CC::File file(fileName.toStdString(), H5CC::Access::rw_require);
    write(file.require_group("histograms1d"), text.toStdString(), histogram1d_.map());
    auto dset = file.require_group("histograms1d").open_dataset(text.toStdString());
    for (auto s : histogram1d_.values())
      dset.write_attribute<double>(s, histogram1d_.get_value(s));
  }
  catch (...)
  {

  }
}

void Analyzer::on_pushSave2D_clicked()
{
  QSettings settings;
  settings.beginGroup("Program");
  auto data_directory = settings.value("data_directory", "").toString();
  QString fileName = CustomSaveFileDialog(this, "Save project",
                                          data_directory, "hdf5 (*.h5)");
  if (fileName.isEmpty())
    return;

  bool ok;
  QString text = QInputDialog::getText(this, "Name histogram",
                                       "Histogram name:", QLineEdit::Normal,
                                       "", &ok);
  if (!ok || text.isEmpty())
    return;

  try
  {
    H5CC::File file(fileName.toStdString(), H5CC::Access::rw_require);
    write(file.require_group("histograms2d"),
          text.toStdString(), hm2d(histogram2d_));
  }
  catch (...)
  {

  }
}

HistMap2D Analyzer::hm2d(const QPlot::HistMap2D& h)
{
  HistMap2D ret;
  for (auto hh : h)
    ret[c2d(hh.first.x, hh.first.y)] = hh.second;
  return ret;
}

void Analyzer::on_pushVary_clicked()
{
  auto fit_type = ui->comboFit->currentText().toStdString();
  if (!reader_ || (fit_type == "none"))
    return;

  auto filter = tests_model_.tests();
  int row = -1;
  auto rows = ui->tableTests->selectionModel()->selectedRows();
  if (rows.size())
    row = rows.front().row();

  if ((row < 0) && (row >= int(filter.size())))
    return;

  DialogVary dv(filter.test(row), this);

  if (dv.exec() != QDialog::Accepted)
    return;

  QSettings settings;
  settings.beginGroup("Program");
  auto data_directory = settings.value("data_directory", "").toString();
  QString fileName = CustomSaveFileDialog(this, "Save results",
                                          data_directory, "hdf5 (*.h5)");
  if (fileName.isEmpty())
    return;

  bool ok;
  QString text = QInputDialog::getText(this, "Name dataset",
                                       "Dataset name:", QLineEdit::Normal,
                                       QString::fromStdString(dv.params().metric),
                                       &ok);
  if (!ok || text.isEmpty())
    return;

  FilterMerits results;
  results.filter = filter;
  results.filter.remove(row);
  results.indvar = dv.params();
  results.fit_type = fit_type;
  results.units = ui->doubleUnits->value();
  results.doit(*reader_, ui->pushMetric1D->text().toStdString());

  H5CC::File file(fileName.toStdString(), H5CC::Access::rw_require);
  H5CC::Group group = file.require_group(text.toStdString());

  group.write_attribute("dataset", reader_->dataset_name());
  group.write_attribute("analysis", reader_->current_analysis());

  results.save(group);
}

void save(const MetricTest& f, QSettings& s)
{
  s.setValue("metric", QString::fromStdString(f.metric));
  s.setValue("enabled", f.enabled);
  s.setValue("round", f.round_before_compare);
  s.setValue("min", f.min);
  s.setValue("max", f.max);
}

void load(MetricTest& f, const QSettings& s)
{
  f.metric = s.value("metric").toString().toStdString();
  f.enabled = s.value("enabled").toBool();
  f.round_before_compare = s.value("round").toBool();
  f.min = s.value("min").toDouble();
  f.max = s.value("max").toDouble();
}

void save(const MetricFilter& f, QSettings& s, QString name)
{
  s.remove(name);
  s.beginWriteArray(name);
  for (size_t i = 0; i < f.size(); ++i)
  {
    s.setArrayIndex(i);
    save(f.test(i), s);
  }
  s.endArray();
}

void load(MetricFilter& f, QSettings& s, QString name)
{
  f.clear();
  int size = s.beginReadArray(name);
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    MetricTest t;
    load(t, s);
    f.add(t);
  }
  s.endArray();
}
