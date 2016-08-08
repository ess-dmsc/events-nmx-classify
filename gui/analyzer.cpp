#include <QSettings>
#include "analyzer.h"
#include "ui_analyzer.h"
#include "tpcMimicVMMx.h"
#include "tpcFindEntry.h"
#include "custom_logger.h"


Analyzer::Analyzer(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Analyzer)
{
  ui->setupUi(this);

  ui->plotHistogram->set_scale_type("Linear");
  ui->plotHistogram->set_plot_style("Step center");
  ui->plotHistogram->set_visible_options(ShowOptions::thickness | ShowOptions::scale | ShowOptions::grid | ShowOptions::save);

  ui->plotSubhist->set_scale_type("Linear");
  ui->plotSubhist->set_plot_style("Step center");
  ui->plotSubhist->set_visible_options(ShowOptions::thickness | ShowOptions::scale | ShowOptions::grid | ShowOptions::save);

  TPC::Event dummy;
  dummy.analyze();
  ui->comboWeights->addItem("none");
  for (auto &c : dummy.analytics_)
    ui->comboWeights->addItem(QString::fromStdString(c.first));

  ui->plot->set_antialiased(false);
  ui->plot->set_scale_type("Linear");
  ui->plot->set_show_legend(true);
  connect(ui->plot, SIGNAL(markers_set(double, double)), this, SLOT(update_gates(double, double)));

  connect(&thread_, SIGNAL(data_ready(std::shared_ptr<EntryList>, double)),
          this, SLOT(update_data(std::shared_ptr<EntryList>, double)));
  connect(&thread_, SIGNAL(histogram_ready(std::shared_ptr<EntryList>,QString)),
          this, SLOT(update_histogram(std::shared_ptr<EntryList >,QString)));
  connect(&thread_, SIGNAL(subhist_ready(std::shared_ptr<EntryList>,QString)),
          this, SLOT(update_subhist(std::shared_ptr<EntryList >,QString)));

  connect(&thread_, SIGNAL(run_complete()), this, SLOT(run_complete()));

  QColor cc (Qt::red);
  cc.setAlpha(64);
  marker_.appearance.default_pen = QPen(cc, 2);

  thread_.set_refresh_frequency(2);

  loadSettings();
}

Analyzer::~Analyzer()
{
  saveSettings();
  delete ui;
}

void Analyzer::enableIO(bool enable)
{
  bool en = reader_ && reader_->event_count() && enable;
  ui->pushStart->setEnabled(en);
  ui->comboWeights->setEnabled(en);
  ui->doubleNormalize->setEnabled(en);
}

void Analyzer::set_new_source(std::shared_ptr<TPC::Reader> r, TPC::Dimensions x, TPC::Dimensions y)
{
  reader_ = r;
  xdims_ = x;
  ydims_ = y;

  int evt_count {0};

  if (reader_)
    evt_count = reader_->event_count();

  clear();
}

void Analyzer::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  ui->comboWeights->setCurrentText(settings.value("weight_type").toString());
}

void Analyzer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("weight_type", ui->comboWeights->currentText());
}


void Analyzer::clear()
{
  resetPlot2D();

  marker_.visible = false;
  ui->plotHistogram->reset_scales();
  ui->plotHistogram->clearGraphs();
  ui->plotHistogram->clearExtras();
  ui->plotHistogram->replot_markers();
  ui->plotHistogram->rescale();
  ui->plotHistogram->redraw();

  ui->plotSubhist->reset_scales();
  ui->plotSubhist->clearGraphs();
  ui->plotSubhist->clearExtras();
  ui->plotSubhist->replot_markers();
  ui->plotSubhist->rescale();
  ui->plotSubhist->redraw();

  ui->progressBar->setValue(0);
}

void Analyzer::on_spinMin_editingFinished()
{
  resetPlot2D();
  thread_.set_bounds(ui->spinMin->value(), ui->spinMax->value());
}

void Analyzer::on_spinMax_editingFinished()
{
  resetPlot2D();
  thread_.set_bounds(ui->spinMin->value(), ui->spinMax->value());
}


void Analyzer::resetPlot2D()
{
  ui->plot->reset_content();
  ui->plot->set_boxes(std::list<MarkerBox2D>());
  ui->plot->set_axes("X (mm)", xdims_.transfrom(0), xdims_.transfrom(xdims_.strips-1),
                     "Y (mm)", xdims_.transfrom(0), xdims_.transfrom(xdims_.strips-1),
                     "Count");
  ui->plot->refresh();
}


void Analyzer::on_pushStart_clicked()
{
  clear();

  if (!reader_|| !reader_->event_count())
    return;

  ui->pushStop->setEnabled(true);
  ui->progressBar->setValue(0);

  emit toggleIO(false);
  thread_.set_bounds(ui->spinMin->value(), ui->spinMax->value());
  thread_.go(reader_, ui->comboWeights->currentText(), ui->doubleNormalize->value());
}

void Analyzer::on_pushStop_clicked()
{
  thread_.terminate();
}

void Analyzer::update_data(std::shared_ptr<EntryList> data, double percent_done)
{
  ui->plot->update_plot(xdims_.strips, xdims_.strips, data);
  ui->progressBar->setValue(percent_done);
}

void Analyzer::run_complete()
{
  emit toggleIO(true);
  ui->pushStop->setEnabled(false);
}

void Analyzer::update_histogram(std::shared_ptr<EntryList> histo,  QString codomain)
{
  std::map<double, double> minima, maxima;
  Calibration calib_ = Calibration();

  ui->plotHistogram->clearGraphs();

  QVector<double> x, y;

  if (histo)
  {
    for (auto &b : *histo)
    {
      double xx = b.first[0];
      double yy = b.second;

      x.push_back(xx);
      y.push_back(yy);

      if (!minima.count(xx) || (minima[xx] > yy))
        minima[xx] = yy;
      if (!maxima.count(xx) || (maxima[xx] < yy))
        maxima[xx] = yy;
    }
  }

  AppearanceProfile profile;
  profile.default_pen = QPen(Qt::darkRed, 2);
  ui->plotHistogram->addGraph(x, y, profile, 8);

  ui->plotHistogram->use_calibrated(/*calib_.valid()*/false);
  ui->plotHistogram->setLabels(codomain, "count");
  ui->plotHistogram->setYBounds(minima, maxima);

//  ui->plotHistogram->setTitle(codomain);

  marker_.visible = true;

  Marker1D left = marker_;
  left.pos.set_bin(ui->spinMin->value(), 8, calib_);

  Marker1D right = marker_;
  right.pos.set_bin(ui->spinMax->value(), 8, calib_);


  ui->plotHistogram->set_block(left, right);

  ui->plotHistogram->replot_markers();
  ui->plotHistogram->redraw();
}

void Analyzer::update_subhist(std::shared_ptr<EntryList> histo,  QString codomain)
{
  std::map<double, double> minima, maxima;
  Calibration calib_ = Calibration();

  ui->plotSubhist->clearGraphs();

  QVector<double> x, y;

  if (histo)
  {
    for (auto &b : *histo)
    {
      double xx = b.first[0];
      double yy = b.second;

      x.push_back(xx);
      y.push_back(yy);

      if (!minima.count(xx) || (minima[xx] > yy))
        minima[xx] = yy;
      if (!maxima.count(xx) || (maxima[xx] < yy))
        maxima[xx] = yy;
    }
  }

  AppearanceProfile profile;
  profile.default_pen = QPen(Qt::darkRed, 2);
  ui->plotSubhist->addGraph(x, y, profile, 8);

  ui->plotSubhist->use_calibrated(/*calib_.valid()*/false);
  ui->plotSubhist->setLabels(codomain, "count");
  ui->plotSubhist->setYBounds(minima, maxima);

  ui->plotSubhist->replot_markers();
  ui->plotSubhist->redraw();
}

void Analyzer::update_gates(double x, double y)
{
  int xx = int(x);
  int yy = int(y);
  int x1 = x - ui->spinBoxWidth->value();
  int x2 = x + ui->spinBoxWidth->value();
  int y1 = y - ui->spinBoxHeight->value();
  int y2 = y + ui->spinBoxHeight->value();

  MarkerBox2D box;
  box.x_c = xdims_.transfrom(x);
  box.x1 = xdims_.transfrom(x1);
  box.x2 = xdims_.transfrom(x2);
  box.y_c = ydims_.transfrom(y);
  box.y1 = ydims_.transfrom(y1);
  box.y2 = ydims_.transfrom(y2);
  box.selectable = false;
  box.selected = true;

  ui->plot->set_range(box);
  ui->plot->replot_markers();

  thread_.set_box_bounds(x1, x2, y1, y2);
}

