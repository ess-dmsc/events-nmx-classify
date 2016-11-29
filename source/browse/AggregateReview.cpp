#include <QSettings>
#include "AggregateReview.h"
#include "ui_AggregateReview.h"
#include "CustomLogger.h"
#include "ExceptionUtil.h"


#include "H5CC_File.h"
#include "histogram_h5.h"
#include "qt_util.h"

AggregateReview::AggregateReview(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::AggregateReview)
{
  ui->setupUi(this);

  ui->plotHistogram->setScaleType("Linear");
  ui->plotHistogram->setPlotStyle("Step center");
  ui->plotHistogram->legend->setVisible(true);
  ui->plotHistogram->legend->setFont(QFont("Helvetica",12));
  ui->plotHistogram->legend->setSelectableParts(QCPLegend::spItems);
  ui->plotHistogram->setLineThickness(2);
  //  ui->plotHistogram->set_visible_options(ShowOptions::zoom | ShowOptions::thickness | ShowOptions::scale | ShowOptions::grid | ShowOptions::save);

  connect(ui->plotHistogram, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)),
          this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));

  connect(ui->searchBox1, SIGNAL(selectionChanged()), this, SLOT(render_selection()));
  connect(ui->searchBox2, SIGNAL(selectionChanged()), this, SLOT(render_selection()));
  connect(ui->searchBox3, SIGNAL(selectionChanged()), this, SLOT(render_selection()));

  make_palette();

  loadSettings();
}

AggregateReview::~AggregateReview()
{
  saveSettings();
  delete ui;
}

void AggregateReview::make_palette()
{
  palette_.clear();

//  QVector<QColor> cols {Qt::red, Qt::blue, Qt::green, Qt::cyan, Qt::magenta};
//  QVector<QColor> cols2 {Qt::darkRed, Qt::darkGreen, Qt::darkBlue,
//                         Qt::darkCyan, Qt::darkMagenta, Qt::darkYellow};

  QVector<QColor> cols;
  for (int i=0; i < 4; ++i)
    cols.push_back(generateColor());


  QVector<QColor> cols2;
  for (int i=0; i < 4; ++i)
    cols2.push_back(generateColor());


//  for (auto c : cols)
//    c.setAlpha(128);

//  for (auto c : cols2)
//    c.setAlpha(128);

  QVector<Qt::PenStyle> styles {Qt::SolidLine, Qt::DashLine,
        Qt::DotLine, Qt::DashDotDotLine, Qt::DashDotLine};

  for (auto s : styles)
    for (auto c : cols)
      palette_.push_back(QPen(c, 1, s));


  for (auto s : styles)
    for (auto c : cols2)
      palette_.push_back(QPen(c, 1, s));

}

void AggregateReview::enableIO(bool enable)
{
  Q_UNUSED(enable);

}

void AggregateReview::loadSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.beginGroup("Review");
  ui->spinMaxHists->setValue(settings.value("max_hists", 10).toInt());
  ui->searchBox1->setFilter(settings.value("filter1", "").toString());
  ui->searchBox2->setFilter(settings.value("filter2", "").toString());
  ui->searchBox3->setFilter(settings.value("filter3", "").toString());
  ui->pushNormalize->setChecked(settings.value("normalize").toBool());

  QString fileName = settings.value("recent_file").toString();
  openFile(fileName);
}

void AggregateReview::saveSettings()
{
  QSettings settings;
  settings.beginGroup("Program");
  settings.beginGroup("Review");
  settings.setValue("max_hists", ui->spinMaxHists->value());
  settings.setValue("filter1", ui->searchBox1->filter());
  settings.setValue("filter2", ui->searchBox2->filter());
  settings.setValue("filter3", ui->searchBox3->filter());
  settings.setValue("normalize", ui->pushNormalize->isChecked());
}

void AggregateReview::render_selection()
{

  std::map<QString, HistMap1D> final;

  for (auto l1 : ui->searchBox1->selection())
    for (auto l2 : ui->searchBox2->selection())
      for (auto l3 : ui->searchBox3->selection())
        final[l1+"/"+l2+"/"+l3] = data_[l1.toStdString()][l2.toStdString()][l3.toStdString()];


  ui->plotHistogram->clearAll();

  ui->labelTotal->setText(QString("%1").arg(final.size()));

  if (int(final.size()) > ui->spinMaxHists->value())
    ui->labelComparator->setText(" >  ");
  else
  {
    ui->labelComparator->setText(" <= ");

    int i=0;
    for (auto f : final)
    {
      HistMap1D f2;

      if (ui->pushNormalize->isChecked())
      {
        double sum = 0;
        for (auto d : f.second)
          sum += d.second;
        double norm = 1.0 / sum * 100.0;
        for (auto d : f.second)
          f2[d.first] = d.second * norm;
      }
      else
        f2 = f.second;


      QPlot::Appearance ap;
      ap.default_pen = palette_.at(i % palette_.size());
      ui->plotHistogram->addGraph(f2, ap, f.first);
      ++i;
    }
  }

  if (ui->pushNormalize->isChecked())
    ui->plotHistogram->setAxisLabels("", "% events");
  else
    ui->plotHistogram->setAxisLabels("", "event counts");

  ui->pushDigDown->setEnabled(
        (ui->searchBox1->selection().size() == 1) &&
        (ui->searchBox2->selection().size() == 1) &&
        (ui->searchBox3->selection().size() == 1));

  ui->plotHistogram->update();
  ui->plotHistogram->zoomOut();
}


void AggregateReview::on_pushOpen_clicked()
{
  ui->pushOpen->setText("");
  QSettings settings;
  settings.beginGroup("Program");
  QString data_directory = settings.value("data_directory", "").toString();

  QString fileName = QFileDialog::getOpenFileName(this, "Load TPC data", data_directory, "HDF5 (*.h5)");
  if (!validateFile(this, fileName, false))
    return;

  openFile(fileName);
}

void AggregateReview::openFile(QString fileName)
{
  QSet<QString> list1, list2, list3;
  try
  {
    H5CC::File file(fileName.toStdString());

    for (auto g1 : file.groups())
    {
      list1.insert(QString::fromStdString(g1));
      auto group1 = file.open_group(g1);
      for (auto g2 : group1.groups())
      {
        list2.insert(QString::fromStdString(g2));
        auto group2 = group1.open_group(g2);
        for (auto dset : group2.datasets())
        {
          list3.insert(QString::fromStdString(dset));
          auto dataset = group2.open_dataset(dset);
          data_[g1][g2][dset] = read(dataset);
        }
      }
    }
  }
  catch (...)
  {
    printException();
  }

  ui->searchBox1->setList(list1.toList());
  ui->searchBox2->setList(list2.toList());
  ui->searchBox3->setList(list3.toList());

  QSettings settings;
  settings.beginGroup("Program");
  settings.beginGroup("Review");

  if (!data_.empty())
  {
    ui->pushOpen->setText("  " + fileName);
    settings.setValue("recent_file", fileName);
  }
  else
  {
    ui->pushOpen->setText("");
    settings.setValue("recent_file", QVariant());
  }

}

void AggregateReview::on_spinMaxHists_editingFinished()
{
  render_selection();
}

void AggregateReview::on_pushNormalize_clicked()
{
  render_selection();
}

void AggregateReview::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
  Q_UNUSED(legend)
  if (item)
  {
    QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
    QStringList list =  plItem->plottable()->name().split("/");
    if (list.size() != 3)
      return;
    ui->searchBox1->Select(list.at(0));
    ui->searchBox2->Select(list.at(1));
    ui->searchBox3->Select(list.at(2));
  }
}


void AggregateReview::on_pushDigDown_clicked()
{
  if (ui->searchBox1->selection().size() != 1)
    return;
  if (ui->searchBox2->selection().size() != 1)
    return;
  if (ui->searchBox3->selection().size() != 1)
    return;

  emit digDownTown(ui->searchBox1->selection().front(),
                   ui->searchBox2->selection().front(),
                   ui->searchBox3->selection().front());
}

void AggregateReview::on_pushButton_clicked()
{
  make_palette();
  render_selection();
}
