#include <QSettings>
#include "Analyzer.h"
#include "ui_Analyzer.h"
#include "CustomLogger.h"


Analyzer::Analyzer(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::Analyzer)
  , subset_params_(QVector<HistParams>())
  , model_(subset_params_)
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
  connect(ui->plot, SIGNAL(markers_set(double, double, bool)), this, SLOT(update_box(double, double, bool)));

  ui->tableBoxes->setModel(&model_);
  ui->tableBoxes->setItemDelegate(&delegate_);
//  ui->tableBoxes->setSelectionModel(&selection_model_);
  ui->tableBoxes->verticalHeader()->hide();
  ui->tableBoxes->horizontalHeader()->setStretchLastSection(true);
  ui->tableBoxes->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableBoxes->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableBoxes->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableBoxes->setSelectionMode(QAbstractItemView::ExtendedSelection);
//  ui->tableBoxes->setEditTriggers(QAbstractItemView::AllEditTriggers);
  ui->tableBoxes->show();

  connect(&model_, SIGNAL(data_changed()), this, SLOT(parameters_changed()));
  connect(&model_, SIGNAL(editing_finished()), this, SLOT(parameters_set()));
  connect(&delegate_, SIGNAL(edit_integer(QModelIndex,QVariant,int)),
          &model_, SLOT(setDataQuietly(QModelIndex,QVariant,int)));

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

  if (enable) {
    ui->tableBoxes->setEditTriggers(QAbstractItemView::AllEditTriggers);
  } else {
    ui->tableBoxes->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }
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

//  parameters_changed();
//  model_.update();
}

void Analyzer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("weight_type", ui->comboWeights->currentText());
  settings.setValue("normalize_by", ui->doubleNormalize->value());

  settings.setValue("projection_min", ui->spinMin->value());
  settings.setValue("projection_max", ui->spinMax->value());
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

  std::map<std::pair<int,int>, double> projection2d;
  QVector<std::map<int, double>> histograms1d;

  auto subset_params = subset_params_;
  subset_params.push_front(HistParams());
  histograms1d.resize(subset_params.size());

  QVector<HistSubset> ret;
  ret.resize(subset_params.size());

  for (auto &ms : data_)
  {
    for (auto &mi : ms.second)
    {
      int x = mi.first.first;
      int y = mi.first.second;

      if ((min <= ms.first) && (ms.first <= max))
      {
        projection2d[mi.first] += mi.second.size();
        std::copy( mi.second.begin(), mi.second.end(), std::inserter( indices, indices.end() ) );
      }

      for (int i=0; i < subset_params.size(); ++i)
        if ((subset_params[i].x1 <= x) && (x <= subset_params[i].x2)
            && (subset_params[i].y1 <= y) && (y <= subset_params[i].y2)
            && (ms.first >= subset_params[i].cutoff))
        {
          histograms1d[i][ms.first] += mi.second.size();
          ret[i].avg += ms.first * mi.second.size();
          ret[i].total_count += mi.second.size();
          ret[i].min += std::min(ret[i].min, static_cast<double>(ms.first));
          ret[i].max += std::min(ret[i].max, static_cast<double>(ms.first));
        }
    }
  }

  EntryList data_list;
  for (auto &point : projection2d)
    data_list.push_back(Entry{{point.first.first,point.first.second}, point.second});
  ui->plot->update_plot(xdims_.strips, ydims_.strips, data_list);
  ui->plot->set_axes("X (mm)", xdims_.transform(0), xdims_.transform(xdims_.strips-1),
                     "Y (mm)", xdims_.transform(0), xdims_.transform(xdims_.strips-1),
                     "Count");

  ui->plot->refresh();

  for (int i=0; i < subset_params.size(); ++i)
  {
    if (ret[i].total_count != 0)
      ret[i].avg /= ret[i].total_count;
    else
      ret[i].avg = 0;

    for (auto &mi : histograms1d[i])
      ret[i].data.push_back(Entry{{mi.first}, mi.second});
  }
  update_histograms(ret);

  emit select_indices(indices);
}

void Analyzer::update_histograms(const MultiHists &all_hists)
{
  std::map<double, double> minima, maxima;

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
    profile.default_pen = QPen(palette_[i % palette_.size()], 2);

    ui->plotHistogram->addGraph(x, y, profile, 8);
  }

  ui->plotHistogram->use_calibrated(/*calib_.valid()*/false);
  ui->plotHistogram->setLabels(ui->comboWeights->currentText(), "count");
  ui->plotHistogram->setYBounds(minima, maxima);

  //  ui->plotHistogram->setTitle(codomain);
  plot_block();
}

void Analyzer::update_box(double x, double y, bool left_mouse)
{
  auto rows = ui->tableBoxes->selectionModel()->selectedRows();
  DBG << "got rows " << rows.size();
  if (rows.size())
  {
    int row = rows.front().row();
    DBG << "row chosen " << row;
    if ((row >= 0) && (row < subset_params_.size()))
    {
      DBG << "left mouse " << left_mouse;
      if (left_mouse)
      {
        subset_params_[row].set_center_x(x);
        subset_params_[row].set_center_y(y);
      }
      subset_params_[row].visible = left_mouse;
      parameters_changed();
      model_.update();
      parameters_set();
    }
  }
}

void Analyzer::parameters_changed()
{
  std::list<MarkerBox2D> boxes;

  for (auto &p : subset_params_)
  {
    MarkerBox2D box;
    box.x_c = xdims_.transform(p.center_x());
    box.x1 = xdims_.transform(p.x1);
    box.x2 = xdims_.transform(p.x2);
    box.y_c = ydims_.transform(p.center_y());
    box.y1 = ydims_.transform(p.y1);
    box.y2 = ydims_.transform(p.y2);
    box.selectable = true;
    box.selected = true;

    boxes.push_back(box);
  }

  ui->plot->set_boxes(boxes);
  ui->plot->replot_markers();
}

void Analyzer::parameters_set()
{
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

void Analyzer::on_pushAddBox_clicked()
{
  HistParams params;

  params.visible = true;
  params.color = palette_[(subset_params_.size() + 1) % palette_.size()];
  params.set_center_x(xdims_.strips / 2);
  params.set_center_y(ydims_.strips / 2);
  params.set_width(xdims_.strips / 2);
  params.set_height(ydims_.strips / 2);

  subset_params_.push_back(params);

  parameters_changed();
  model_.update();
  parameters_set();
}

void Analyzer::on_spinMin_editingFinished()
{
  make_projections();
}

void Analyzer::on_spinMax_editingFinished()
{
  make_projections();
}

void Analyzer::on_spinMin_valueChanged(int arg1)
{
  plot_block();
}

void Analyzer::on_spinMax_valueChanged(int arg1)
{
  plot_block();
}

void Analyzer::plot_block()
{
  Calibration calib_ = Calibration();

  Marker1D marker_;
  marker_.visible = true;
  QColor cc (Qt::red);
  cc.setAlpha(64);
  marker_.appearance.default_pen = QPen(cc, 2);

  Marker1D left = marker_;
  left.pos.set_bin(ui->spinMin->value(), 8, calib_);

  Marker1D right = marker_;
  right.pos.set_bin(ui->spinMax->value(), 8, calib_);

  ui->plotHistogram->set_block(left, right);

  ui->plotHistogram->replot_markers();
  ui->plotHistogram->redraw();
}
