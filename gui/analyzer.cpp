#include <QSettings>
#include "analyzer.h"
#include "ui_analyzer.h"
#include "CustomLogger.h"


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

  NMX::Record dummy;
  dummy.analyze();
  ui->comboWeights->addItem("none");
  for (auto &c : dummy.categories())
    ui->comboWeights->addItem(QString::fromStdString(c));

  ui->plot->set_antialiased(false);
  ui->plot->set_scale_type("Linear");
  ui->plot->set_show_legend(true);
  connect(ui->plot, SIGNAL(markers_set(double, double)), this, SLOT(update_box(double, double)));

  connect(&thread_, SIGNAL(data_ready(std::shared_ptr<EntryList>, double)),
          this, SLOT(update_data(std::shared_ptr<EntryList>, double)));
  connect(&thread_, SIGNAL(hists_ready(std::shared_ptr<MultiHists>)),
          this, SLOT(update_histograms(std::shared_ptr<MultiHists>)));


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

void Analyzer::set_params(std::map<std::string, double> params)
{
  params_ = params;
}

void Analyzer::set_new_source(std::shared_ptr<NMX::FileHDF5> r, NMX::Dimensions x, NMX::Dimensions y)
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
  ui->doubleNormalize->setValue(settings.value("normalize_by", 1).toDouble());
  ui->spinBoxWidth->setValue(settings.value("box_width").toInt());
  ui->spinBoxHeight->setValue(settings.value("box_height").toInt());
  box_x_ = settings.value("box_x", xdims_.strips / 2).toInt();
  box_y_ = settings.value("box_y", ydims_.strips / 2).toInt();
}

void Analyzer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("weight_type", ui->comboWeights->currentText());
  settings.setValue("normalize_by", ui->doubleNormalize->value());
  settings.setValue("box_width", ui->spinBoxWidth->value());
  settings.setValue("box_height", ui->spinBoxHeight->value());
  settings.setValue("box_x", box_x_);
  settings.setValue("box_y", box_y_);
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
  ui->plot->set_axes("X (mm)", xdims_.transform(0), xdims_.transform(xdims_.strips-1),
                     "Y (mm)", xdims_.transform(0), xdims_.transform(xdims_.strips-1),
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
  update_gates();
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

void Analyzer::update_histograms(std::shared_ptr<MultiHists> all_hists)
{
  std::map<double, double> minima, maxima;

  std::map<double, double> minima_sub, maxima_sub;

  Calibration calib_ = Calibration();

  ui->plotHistogram->clearGraphs();
  ui->plotSubhist->clearGraphs();

  if (all_hists)
  {
    for (int i=0; i < all_hists->size(); ++i)
    {
      QVector<double> x, y;

      for (auto &b : (*all_hists)[i].data)
      {
        double xx = b.first[0];
        double yy = b.second;

        x.push_back(xx);
        y.push_back(yy);

        if (i == 0)
        {
          if (!minima.count(xx) || (minima[xx] > yy))
            minima[xx] = yy;
          if (!maxima.count(xx) || (maxima[xx] < yy))
            maxima[xx] = yy;
        }
        else
        {
          if (!minima_sub.count(xx) || (minima_sub[xx] > yy))
            minima_sub[xx] = yy;
          if (!maxima_sub.count(xx) || (maxima_sub[xx] < yy))
            maxima_sub[xx] = yy;
        }
      }
      AppearanceProfile profile;
      profile.default_pen = QPen(Qt::darkRed, 2);

      if (i == 0)
        ui->plotHistogram->addGraph(x, y, profile, 8);
      else
        ui->plotSubhist->addGraph(x, y, profile, 8);
    }
  }

  ui->plotHistogram->use_calibrated(/*calib_.valid()*/false);
  ui->plotHistogram->setLabels(ui->comboWeights->currentText(), "count");
  ui->plotHistogram->setYBounds(minima, maxima);

  ui->plotSubhist->use_calibrated(/*calib_.valid()*/false);
  ui->plotSubhist->setLabels(ui->comboWeights->currentText(), "count");
  ui->plotSubhist->setYBounds(minima_sub, maxima_sub);

//  ui->plotHistogram->setTitle(codomain);

  marker_.visible = true;

  Marker1D left = marker_;
  left.pos.set_bin(ui->spinMin->value(), 8, calib_);

  Marker1D right = marker_;
  right.pos.set_bin(ui->spinMax->value(), 8, calib_);

  ui->plotHistogram->set_block(left, right);

  ui->plotHistogram->replot_markers();
  ui->plotHistogram->redraw();

  ui->plotSubhist->replot_markers();
  ui->plotSubhist->redraw();
}

void Analyzer::update_box(double x, double y)
{
  box_x_ = x;
  box_y_ = y;

  update_gates();
}

void Analyzer::update_gates()
{
  QVector<HistParams> params;
  params.resize(2);

  params[1].x1 = box_x_ - ui->spinBoxWidth->value() / 2;
  params[1].x2 = box_x_ + ui->spinBoxWidth->value() / 2;
  params[1].y1 = box_y_ - ui->spinBoxHeight->value() / 2;
  params[1].y2 = box_y_ + ui->spinBoxHeight->value() / 2;

  MarkerBox2D box;
  box.x_c = xdims_.transform(box_x_);
  box.x1 = xdims_.transform(params[1].x1);
  box.x2 = xdims_.transform(params[1].x2);
  box.y_c = ydims_.transform(box_y_);
  box.y1 = ydims_.transform(params[1].y1);
  box.y2 = ydims_.transform(params[1].y2);
  box.selectable = false;
  box.selected = true;

  ui->plot->set_range(box);
  ui->plot->replot_markers();

  thread_.request_hists(params);
}

