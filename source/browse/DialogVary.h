#pragma once

#include <vector>
#include <QDialog>
#include "Filter.h"

namespace Ui {
class DialogVary;
}

class DialogVary : public QDialog
{
  Q_OBJECT

public:
  explicit DialogVary(IndepVariable pars, QWidget *parent = 0);
  ~DialogVary();

  IndepVariable params() const;

  double start() const;
  double end() const;
  double step() const;
  double window() const;

  bool vary_min() const;
  bool vary_max() const;

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();

  void on_radioBoth_toggled(bool checked);

private:
  Ui::DialogVary *ui;
};
