#pragma once

#include <vector>
#include <QDialog>

namespace Ui {
class DialogVary;
}

class DialogVary : public QDialog
{
  Q_OBJECT

public:
  explicit DialogVary(QString metric, double min, double max, QWidget *parent = 0);
  ~DialogVary();

  double start() const;
  double end() const;
  double step() const;

  bool vary_min() const;
  bool vary_max() const;

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();

private:
  Ui::DialogVary *ui;
};
