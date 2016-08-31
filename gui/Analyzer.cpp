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
  params.color = palette_[histograms1d_.size() % palette_.size()];
  params.set_x(0, 250);
  params.set_y(0, 250);
  histograms1d_.push_back(params);

  ui->plotHistogram->set_scale_type("Linear");
  ui->plotHistogram->set_plot_style("Step center");
  ui->plotHistogram->set_visible_options(ShowOptions::zoom | ShowOptions::thickness | ShowOptions::scale | ShowOptions::grid | ShowOptions::save);

  NMX::Event dummy;
  dummy.analyze();
  ui->comboWeightsZ->addItem("none");
  for (auto &c : dummy.categories())
  {
    ui->comboWeightsX->addItem(QString::fromStdString(c));
    ui->comboWeightsY->addItem(QString::fromStdString(c));
    ui->comboWeightsZ->addItem(QString::fromStdString(c));
  }

  ui->plot->set_antialiased(false);
  ui->plot->set_scale_type("Linear");
  ui->plot->set_show_legend(true);
  connect(ui->plot, SIGNAL(markers_set(double, double, bool)), this, SLOT(update_box(double, double, bool)));

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
  ui->doubleNormalize->setEnabled(en);

  if (enable) {
    ui->tableBoxes->setEditTriggers(QAbstractItemView::AllEditTriggers);
  } else {
    ui->tableBoxes->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }
}

void Analyzer::set_new_source(std::shared_ptr<NMX::FileHDF5> r)
{
  reader_ = r;
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
  ui->doubleNormalize->setValue(settings.value("normalize_by", 1).toDouble());

  ui->spinMin->setValue(settings.value("projection_min", 0).toInt());
  ui->spinMax->setValue(settings.value("projection_max", 1000000).toInt());
}

void Analyzer::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.setValue("weight_type_x", ui->comboWeightsX->currentText());
  settings.setValue("weight_type_y", ui->comboWeightsY->currentText());
  settings.setValue("weight_type_z", ui->comboWeightsZ->currentText());
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

  auto xx      = reader_->get_category(ui->comboWeightsX->currentText().toStdString());
  auto yy      = reader_->get_category(ui->comboWeightsY->currentText().toStdString());
  auto weights = reader_->get_category(ui->comboWeightsZ->currentText().toStdString());

  for (size_t eventID = 0; eventID < reader_->num_analyzed(); ++eventID)
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

  for (auto &i : histograms1d_)
    i.reset_data();

  make_projections();
}

void Analyzer::make_projections()
{
  std::set<size_t> indices;

  int min = ui->spinMin->value();
  int max = ui->spinMax->value();

  std::map<std::pair<int,int>, double> projection2d;

  int xmax = 0;
  int ymax = 0;

  for (auto &ms : data_)
  {
    auto &z = ms.first;
    for (auto &mi : ms.second)
    {
      int x = mi.first.first;
      int y = mi.first.second;

      xmax = std::max(xmax, x);
      ymax = std::max(ymax, y);

      if ((min <= z) && (z <= max))
      {
        projection2d[mi.first] += mi.second.size();
        std::copy( mi.second.begin(), mi.second.end(), std::inserter( indices, indices.end() ) );
      }

      for (auto &i : histograms1d_)
        i.add_to_hist(x, y, z, mi.second.size());
    }
  }

  for (auto &i : histograms1d_)
    i.close_data();

  EntryList data_list;
  for (auto &point : projection2d)
    data_list.push_back(Entry{{point.first.first,point.first.second}, point.second});
  ui->plot->update_plot(xmax, ymax, data_list);
  ui->plot->set_axes(ui->comboWeightsX->currentText(), 0, xmax,
                     ui->comboWeightsY->currentText(), 0, ymax,
                     "Count");
  ui->plot->refresh();

  update_histograms();

  emit select_indices(indices);
}

void Analyzer::update_histograms()
{
  std::map<double, double> minima, maxima;

  ui->plotHistogram->clearGraphs();

  for (int i=0; i < histograms1d_.size(); ++i)
  {
    if (!histograms1d_[i].visible)
      continue;

    QVector<double> x, y;

    for (auto &b : histograms1d_[i].data())
    {
      double xx = b.first;
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

  ui->plotHistogram->setLabels(ui->comboWeightsZ->currentText(), "count");
  ui->plotHistogram->setYBounds(minima, maxima);

  plot_block();
}

void Analyzer::update_box(double x, double y, bool left_mouse)
{
  auto rows = ui->tableBoxes->selectionModel()->selectedRows();
  if (rows.size())
  {
    int row = rows.front().row();
//    DBG << "change for row " << row << " " << x << " " << y;
    if ((row >= 0) && (row < histograms1d_.size()))
    {
      if (left_mouse)
      {
        histograms1d_[row].set_center_x(static_cast<int64_t>(x));
        histograms1d_[row].set_center_y(static_cast<int64_t>(y));
      }
      histograms1d_[row].visible = left_mouse;
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
  std::list<MarkerBox2D> boxes;

  int i=0;
  for (auto &p : histograms1d_)
  {
    if (!p.visible)
      continue;

    MarkerBox2D box;
    box.x1 = p.x1();
    box.x2 = p.x2();
    box.y1 = p.y1();
    box.y2 = p.y2();
    box.selectable = false;
//    box.selected = true;
    box.fill = box.border = p.color;
    box.fill.setAlpha(48);
    box.label = QString::number(i);

    boxes.push_back(box);
    i++;
  }

  ui->plot->set_boxes(boxes);
  ui->plot->replot_markers();
}

void Analyzer::parameters_set()
{
  make_projections();
}

void Analyzer::on_comboWeightsX_currentIndexChanged(const QString &arg1)
{
  rebuild_data();
}

void Analyzer::on_comboWeightsY_currentIndexChanged(const QString &arg1)
{
  rebuild_data();
}

void Analyzer::on_comboWeightsZ_currentIndexChanged(const QString &arg1)
{
  rebuild_data();
}

void Analyzer::on_doubleNormalize_editingFinished()
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
  Marker1D marker_;
  marker_.visible = true;//ui->checkShowUngated->isChecked();
  QColor cc (Qt::darkGray);
  cc.setAlpha(64);
  marker_.appearance.default_pen = QPen(cc, 2);

  Marker1D left = marker_;
  left.pos = ui->spinMin->value();

  Marker1D right = marker_;
  right.pos = ui->spinMax->value();

  ui->plotHistogram->set_block(left, right);

  ui->plotHistogram->replot_markers();
  ui->plotHistogram->redraw();
}


