#include <algorithm>
#include <QPaintEvent>
#include <QPainter>
#include <QFileInfo>
#include "DialogVary.h"
#include "ui_DialogVary.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

DialogVary::DialogVary(IndepVariable pars, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogVary)
{
  ui->setupUi(this);
  ui->labelWidth->setVisible(false);
  ui->doubleWidth->setVisible(false);

  ui->labelMetric->setText(QString::fromStdString(pars.metric));
  ui->radioMax->setChecked(true);

  ui->doubleMin->setRange(pars.start, pars.end);
  ui->doubleMin->setValue(pars.start);

  ui->doubleMax->setRange(pars.start, pars.end);
  ui->doubleMax->setValue(pars.end);

  ui->doubleStep->setRange(0, pars.end-pars.start+1);
  ui->doubleStep->setValue(1);

  ui->doubleWidth->setRange(1, pars.end-pars.start+1);
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

IndepVariable DialogVary::params() const
{
  IndepVariable ret;
  ret.metric = ui->labelMetric->text().toStdString();
  ret.start = ui->doubleMin->value();
  ret.end = ui->doubleMax->value();
  ret.step = ui->doubleStep->value();
  ret.width = ui->doubleStep->value();
  ret.vary_min = (ui->radioMin->isChecked() || ui->radioBoth->isChecked());
  ret.vary_max = (ui->radioMax->isChecked() || ui->radioBoth->isChecked());
  return ret;
}

void DialogVary::on_radioBoth_toggled(bool checked)
{
  ui->labelWidth->setVisible(ui->radioBoth->isChecked());
  ui->doubleWidth->setVisible(ui->radioBoth->isChecked());
}
