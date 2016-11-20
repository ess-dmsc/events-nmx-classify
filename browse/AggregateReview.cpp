#include <QSettings>
#include "AggregateReview.h"
#include "ui_AggregateReview.h"
#include "CustomLogger.h"


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
  ui->plotHistogram->legend->setFont(QFont("Helvetica",9));
  //  ui->plotHistogram->set_visible_options(ShowOptions::zoom | ShowOptions::thickness | ShowOptions::scale | ShowOptions::grid | ShowOptions::save);

  connect(ui->searchBox1, SIGNAL(selectionChanged()), this, SLOT(render_selection()));
  connect(ui->searchBox2, SIGNAL(selectionChanged()), this, SLOT(render_selection()));
  connect(ui->searchBox3, SIGNAL(selectionChanged()), this, SLOT(render_selection()));

  loadSettings();
}

AggregateReview::~AggregateReview()
{
  saveSettings();
  delete ui;
}

void AggregateReview::enableIO(bool enable)
{
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
      double sum = 0;
      for (auto d : f.second)
        sum += d.second;

      HistMap1D f2;
      for (auto d : f.second)
        f2[d.first] = d.second / sum;

      QPlot::Appearance ap;
      ap.default_pen = QPen(generateColor());
      ui->plotHistogram->addGraph(f2, ap, f.first);
      ++i;
    }
  }

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

  H5CC::File file(fileName.toStdString());
  ui->pushOpen->setText(fileName);

  QSet<QString> list1, list2, list3;

  for (auto g1 : file.groups())
  {
    list1.insert(QString::fromStdString(g1));
    auto group1 = file.group(g1);
    for (auto g2 : group1.groups())
    {
      list2.insert(QString::fromStdString(g2));
      auto group2 = group1.group(g2);
      for (auto dset : group2.datasets())
      {
        list3.insert(QString::fromStdString(dset));
        auto dataset = group2.open_dataset(dset);
        data_[g1][g2][dset] = read(dataset);
      }
    }
  }

  ui->searchBox1->setList(list1.toList());
  ui->searchBox2->setList(list2.toList());
  ui->searchBox3->setList(list3.toList());
}

void AggregateReview::on_spinMaxHists_editingFinished()
{
  render_selection();
}
