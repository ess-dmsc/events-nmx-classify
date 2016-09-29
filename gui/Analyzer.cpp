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

  ui->plot2D->set_antialiased(false);
  ui->plot2D->set_scale_type("Linear");
  ui->plot2D->set_show_legend(true);
  connect(ui->plot2D, SIGNAL(markers_set(double, double, bool)), this, SLOT(update_box(double, double, bool)));

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

void Analyzer::set_new_source(std::shared_ptr<NMX::FileHDF5> r)
{
  reader_ = r;

  auto weight_x = ui->comboWeightsX->currentText();
  auto weight_y = ui->comboWeightsY->currentText();
  auto weight_z = ui->comboWeightsZ->currentText();

  populate_combos();

  ui->comboWeightsX->setCurrentText(weight_x);
  ui->comboWeightsY->setCurrentText(weight_y);
  ui->comboWeightsZ->setCurrentText(weight_z);

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

  ui->labelX->setText("   " + QString::fromStdString(reader_->metric_description(weight_x)));
  ui->labelY->setText("   " + QString::fromStdString(reader_->metric_description(weight_y)));
  ui->labelZ->setText("   " + QString::fromStdString(reader_->metric_description(weight_z)));

  if (xx.size() != yy.size())
  {
    make_projections();
    return;
  }

  xx_norm = normalizer(xx);
  yy_norm = normalizer(yy);
  zz_norm = normalizer(zz);

//  DBG << weight_x << " normalized by " << xx_norm;
//  DBG << weight_y << " normalized by " << yy_norm;
//  DBG << weight_z << " normalized by " << zz_norm;

  for (size_t eventID = 0; eventID < reader_->num_analyzed(); ++eventID)
  {
    if ((eventID >= xx.size()) || (eventID >= yy.size()))
      continue;

    double quality {0};
    if (eventID < zz.size())
      quality = zz.at(eventID).as_float() / zz_norm;

    std::pair<int,int> pos{int(xx.at(eventID).as_float() / xx_norm),
                           int(yy.at(eventID).as_float() / yy_norm)};

    data_[int(quality)][pos].push_back(eventID);
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

  std::map<std::pair<int32_t,int32_t>, double> projection2d;

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
        projection2d[mi.first] += mi.second.size();
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

  EntryList data_list;
  for (auto &point : projection2d)
    data_list.push_back(Entry{{point.first.first - xmin, point.first.second - ymin}, point.second});
  ui->plot2D->update_plot(xmax-xmin, ymax-ymin, data_list);
  ui->plot2D->set_axes(ui->comboWeightsX->currentText(), xmin * xx_norm, xmax * xx_norm,
                     ui->comboWeightsY->currentText(), ymin * yy_norm, ymax * yy_norm,
                     "Count");
  ui->plot2D->refresh();

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

  ui->plotHistogram->setTitle(ui->comboWeightsZ->currentText()
                              + "  (normalized by: " + QString::number(zz_norm) + ")");

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

  ui->plot2D->set_boxes(boxes);
  ui->plot2D->replot_markers();
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
  Marker1D marker_;
  marker_.visible = true;//ui->checkShowUngated->isChecked();
  QColor cc (Qt::darkGray);
  cc.setAlpha(64);
  marker_.appearance.default_pen = QPen(cc, 2);

  Marker1D left = marker_;
  left.pos = ui->spinMinZ->value();

  Marker1D right = marker_;
  right.pos = ui->spinMaxZ->value();

  ui->plotHistogram->set_block(left, right);

  ui->plotHistogram->replot_markers();
  ui->plotHistogram->redraw();
}

double Analyzer::normalizer(const std::vector<Variant> &data)
{
  double minimum{std::numeric_limits<double>::max()};
  double maximum{std::numeric_limits<double>::min()};
  for (auto &d : data)
  {
    if ((d.type() == Variant::code::type_null) ||
        (d.type() == Variant::code::type_menu))
      continue;

    double val = d.as_float();

    minimum = std::min(val, minimum);
    maximum = std::max(val, maximum);
  }

  if (minimum >= maximum)
    return 1;

  double diff = maximum - minimum;

  if (diff <= 1.0)
    return 0.01;

  if (diff > 10000.0)
  {
    int order_of = std::floor(std::log10(std::abs(diff)));
    return pow(10, order_of - 2);
  }

  return 1;
}

