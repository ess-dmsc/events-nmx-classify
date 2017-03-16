#include <algorithm>
#include <QPaintEvent>
#include <QPainter>
#include <QFileInfo>
#include "DialogVary.h"
#include "ui_DialogVary.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

DialogVary::DialogVary(QString metric, double min, double max, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogVary)
{
  ui->setupUi(this);
  ui->labelWidth->setVisible(false);
  ui->doubleWidth->setVisible(false);


  ui->labelMetric->setText(metric);
  ui->radioMax->setChecked(true);

  ui->doubleMin->setRange(min, max);
  ui->doubleMin->setValue(min);

  ui->doubleMax->setRange(min, max);
  ui->doubleMax->setValue(max);

  ui->doubleStep->setRange(0, max-min+1);
  ui->doubleStep->setValue(1);

  ui->doubleWidth->setRange(1, max-min+1);
  ui->doubleWidth->setValue(1);
}

DialogVary::~DialogVary()
{
  delete ui;
}

void DialogVary::on_buttonBox_accepted()
{
  emit accepted();
}

void DialogVary::on_buttonBox_rejected()
{
  emit accepted();
}

double DialogVary::start() const
{
  return ui->doubleMin->value();
}

double DialogVary::end() const
{
  return ui->doubleMax->value();
}

double DialogVary::step() const
{
  return ui->doubleStep->value();
}

double DialogVary::window() const
{
  return ui->doubleWidth->value();
}

bool DialogVary::vary_min() const
{
  return (ui->radioMin->isChecked() || ui->radioBoth->isChecked());
}

bool DialogVary::vary_max() const
{
  return (ui->radioMax->isChecked() || ui->radioBoth->isChecked());
}


void DialogVary::on_radioBoth_toggled(bool checked)
{
  ui->labelWidth->setVisible(ui->radioBoth->isChecked());
  ui->doubleWidth->setVisible(ui->radioBoth->isChecked());
}
