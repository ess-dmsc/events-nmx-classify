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

  NMX::Event dummy;
  dummy.analyze();
  ui->comboWeights->addItem("none");
  for (auto &c : dummy.categories())
    ui->comboWeights->addItem(QString::fromStdString(c));

  ui->plot->set_antialiased(false);
  ui->plot->set_scale_type("Linear");
  ui->plot->set_show_legend(true);
  connect(ui->plot, SIGNAL(markers_set(double, double)), this, SLOT(update_box(double, double)));

  QColor cc (Qt::red);
  cc.setAlpha(64);
  marker_.appearance.default_pen = QPen(cc, 2);

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
  ui->comboWeights->setEnabled(en);
  ui->doubleNormalize->setEnabled(en);
}

void Analyzer::set_new_source(std::shared_ptr<NMX::FileHDF5> r, NMX::Dimensions x, NMX::Dimensions y)
{
  reader_ = r;
  xdims_ = x;
  ydims_ = y;

  rebuild_data();
}

void Analyzer::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  ui->comboWeights->setCurrentText(settings.value("weight_type").toString());
  ui->doubleNormalize->setValue(settings.value("normalize_by", 1).toDouble());

  ui->spinMin->setValue(settings.value("projection_min", 0).toInt());
  ui->spinMax->setValue(settings.value("projection_max", 1000000).toInt());

  ui->spinBoxWidth->setValue(settings.value("box_width").toInt());
  ui->spinBoxHeight->setValue(settings.value("box_height").toInt());
  ui->spinBoxX->setValue(settings.value("box_x", xdims_.strips / 2).toInt());
  ui->spinBoxY->setValue(settings.value("box_y", ydims_.strips / 2).toInt());

  update_gates();
}

void Analyzer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("weight_type", ui->comboWeights->currentText());
  settings.setValue("normalize_by", ui->doubleNormalize->value());

  settings.setValue("projection_min", ui->spinMin->value());
  settings.setValue("projection_max", ui->spinMax->value());

  settings.setValue("box_width", ui->spinBoxWidth->value());
  settings.setValue("box_height", ui->spinBoxHeight->value());
  settings.setValue("box_x", ui->spinBoxX->value());
  settings.setValue("box_y", ui->spinBoxY->value());
}

void Analyzer::rebuild_data()
{
  if (!reader_|| !reader_->event_count())
    return;

  data_.clear();

  double normalize_by = ui->doubleNormalize->value();

  auto weights = reader_->get_category(ui->comboWeights->currentText().toStdString());
  auto xx      = reader_->get_category("X_entry_strip");
  auto yy      = reader_->get_category("Y_entry_strip");

  int evt_count = reader_->num_analyzed();
  for (size_t eventID = 0; eventID < evt_count; ++eventID)
  {
    double quality {0};
    if (eventID < weights.size())
      quality = weights.at(eventID);
    if (normalize_by != 0)
      quality /= normalize_by;

    if ((eventID >= xx.size()) || (eventID >= yy.size()))
      continue;

    if ((xx.at(eventID) < 0) ||
        (yy.at(eventID) < 0))
      continue;

    std::pair<int,int> pos{xx.at(eventID), yy.at(eventID)};

    data_[int(quality)][pos].push_back(eventID);
  }

  make_projections();
}

void Analyzer::make_projections()
{

  std::set<size_t> indices;

  int min = ui->spinMin->value();
  int max = ui->spinMax->value();

  std::map<std::pair<int,int>, double> projection;
  QVector<std::map<int, double>> histograms;
  histograms.resize(subset_params_.size());

  QVector<HistSubset> ret;
  ret.resize(subset_params_.size());

  for (auto &ms : data_)
  {
    bool add_toi_projection = ((min <= ms.first) && (ms.first <= max));
    for (auto &mi : ms.second)
    {
      if (add_toi_projection)
      {
        projection[mi.first] += mi.second.size();
        std::copy( mi.second.begin(), mi.second.end(), std::inserter( indices, indices.end() ) );
      }

      int x = mi.first.first;
      int y = mi.first.second;

      for (int i=0; i < subset_params_.size(); ++i)
        if ((subset_params_[i].x1 <= x) && (x <= subset_params_[i].x2)
            && (subset_params_[i].y1 <= y) && (y <= subset_params_[i].y2)
            && (ms.first >= subset_params_[i].cutoff))
        {
          histograms[i][ms.first] += mi.second.size();
          ret[i].avg += ms.first * mi.second.size();
          ret[i].total_count += mi.second.size();
          ret[i].min += std::min(ret[i].min, static_cast<double>(ms.first));
          ret[i].max += std::min(ret[i].max, static_cast<double>(ms.first));
        }
    }
  }

  EntryList data_list;
  for (auto &point : projection)
    data_list.push_back(Entry{{point.first.first,point.first.second}, point.second});
  ui->plot->update_plot(xdims_.strips, ydims_.strips, data_list);
  ui->plot->set_axes("X (mm)", xdims_.transform(0), xdims_.transform(xdims_.strips-1),
                     "Y (mm)", xdims_.transform(0), xdims_.transform(xdims_.strips-1),
                     "Count");

  ui->plot->refresh();

  for (int i=0; i < subset_params_.size(); ++i)
  {
    if (ret[i].total_count != 0)
      ret[i].avg /= ret[i].total_count;
    else
      ret[i].avg = 0;

    for (auto &mi : histograms[i])
      ret[i].data.push_back(Entry{{mi.first}, mi.second});
  }
  update_histograms(ret);

  emit select_indices(indices);
}

void Analyzer::update_histograms(const MultiHists &all_hists)
{
  QVector<QColor> palette {Qt::black, Qt::darkRed, Qt::darkGreen, Qt::darkCyan, Qt::darkYellow, Qt::darkMagenta, Qt::darkBlue, Qt::red, Qt::blue};

  std::map<double, double> minima, maxima;

  Calibration calib_ = Calibration();

  ui->plotHistogram->clearGraphs();

  for (int i=0; i < all_hists.size(); ++i)
  {
    QVector<double> x, y;

    for (auto &b : all_hists[i].data)
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
    AppearanceProfile profile;
    profile.default_pen = QPen(palette[i % palette.size()], 2);

    ui->plotHistogram->addGraph(x, y, profile, 8);
  }

  ui->plotHistogram->use_calibrated(/*calib_.valid()*/false);
  ui->plotHistogram->setLabels(ui->comboWeights->currentText(), "count");
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

void Analyzer::update_box(double x, double y)
{
  ui->spinBoxX->setValue(x);
  ui->spinBoxY->setValue(x);
  update_gates();
}

void Analyzer::update_gates()
{
  subset_params_.clear();
  subset_params_.resize(2);

  subset_params_[1].x1 = ui->spinBoxX->value() - ui->spinBoxWidth->value() / 2;
  subset_params_[1].x2 = ui->spinBoxX->value() + ui->spinBoxWidth->value() / 2;
  subset_params_[1].y1 = ui->spinBoxY->value() - ui->spinBoxHeight->value() / 2;
  subset_params_[1].y2 = ui->spinBoxY->value() + ui->spinBoxHeight->value() / 2;

  MarkerBox2D box;
  box.x_c = xdims_.transform(ui->spinBoxX->value());
  box.x1 = xdims_.transform(subset_params_[1].x1);
  box.x2 = xdims_.transform(subset_params_[1].x2);
  box.y_c = ydims_.transform(ui->spinBoxY->value());
  box.y1 = ydims_.transform(subset_params_[1].y1);
  box.y2 = ydims_.transform(subset_params_[1].y2);
  box.selectable = false;
  box.selected = true;

  ui->plot->set_range(box);
  ui->plot->replot_markers();
  make_projections();
}

void Analyzer::on_comboWeights_currentIndexChanged(const QString &arg1)
{
  rebuild_data();
}

void Analyzer::on_doubleNormalize_editingFinished()
{
  rebuild_data();
}

void Analyzer::on_spinBoxX_valueChanged(int arg1)
{
  update_gates();
}

void Analyzer::on_spinBoxY_valueChanged(int arg1)
{
  update_gates();
}

void Analyzer::on_spinBoxWidth_valueChanged(int arg1)
{
  update_gates();
}

void Analyzer::on_spinBoxHeight_valueChanged(int arg1)
{
  update_gates();
}

void Analyzer::on_spinMin_valueChanged(int arg1)
{
  make_projections();
}

void Analyzer::on_spinMax_valueChanged(int arg1)
{
  make_projections();
}
