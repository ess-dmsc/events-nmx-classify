#include <QSettings>
#include "ViewRecord.h"
#include "ui_ViewRecord.h"


ViewRecord::ViewRecord(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ViewRecord)
{
  ui->setupUi(this);

  ui->plotRecord->set_antialiased(false);
  ui->plotRecord->set_scale_type("Linear");
  ui->plotRecord->set_gradient("YlGnBu5");
  ui->plotRecord->set_show_legend(true);
  ui->plotRecord->set_zoom_drag(Qt::Horizontal);
}

ViewRecord::~ViewRecord()
{
  delete ui;
}

void ViewRecord::set_title(QString title)
{
  ui->labelTitle->setText(title);
}

void ViewRecord::set_show_raw(bool show_raw)
{
  show_raw_ = show_raw;
  display_current_record();
}

void ViewRecord::set_overlay_type(QString overlay_type)
{
  overlay_type_ = overlay_type;
  display_current_record();
}

void ViewRecord::clear()
{
  ui->plotRecord->reset_content();
  ui->plotRecord->set_boxes(std::list<MarkerBox2D>());
  ui->plotRecord->refresh();

  moving_.visible = false;
}


void ViewRecord::display_record(const NMX::Record &record)
{
  record_ = record;
  display_current_record();
}

void ViewRecord::display_current_record()
{
  clear();

  ui->plotRecord->set_axes("Position (strip)", record_.strip_start(), record_.strip_end(),
                           "Time bin",                          0, record_.time_end(),
                           "Charge");

  if (show_raw_)
    ui->plotRecord->update_plot(record_.strip_span(), record_.time_end() + 1, make_list());
  else
    ui->plotRecord->update_plot(record_.strip_span(), record_.time_end() + 1, EntryList());

  auto metrics = record_.metrics();

  auto overlay = make_overlay();
  int stripi = metrics["entry_strip"].value.as_int(-1);
  if (stripi >= 0)
  {
    int timei = metrics["entry_time"].value.as_int(-1);

    MarkerBox2D box;
    box.selectable = false;
    box.x1 = stripi - 0.2;
    box.x2 = stripi + 0.2;
    box.y1  = timei - 0.2;
    box.y2  = timei + 0.2;
    box.border = Qt::yellow;
    box.fill = Qt::darkYellow;
    overlay.push_back(box);
  }
  ui->plotRecord->set_boxes(overlay);
  ui->plotRecord->replot_markers();
  ui->plotRecord->zoom_out();
}

EntryList ViewRecord::make_list()
{
  EntryList data;

  NMX::Record r;
  if (noneg_)
    r = record_.suppress_negatives();
  else
    r = record_;

  for (auto &i : r.valid_strips())
  {
    auto strip = r.get_strip(i);
    int stripi = i - r.strip_start();
    for (int tb=strip.bin_start(); tb <= strip.bin_end(); ++tb)
      if (strip.value(tb))
        data.push_back(Entry{{stripi,tb}, strip.value(tb)});
  }
  return data;
}

std::list<MarkerBox2D> ViewRecord::make_overlay()
{
  std::list<MarkerBox2D> ret;

  for (auto &i : record_.get_points(overlay_type_.toStdString()))
  {
    int stripi = i.first;

    MarkerBox2D box;
    box.selectable = false;
    box.x1 = stripi - 0.45;
    box.x2 = stripi + 0.45;
    box.y1 = i.second - 0.45;
    box.y2 = i.second + 0.45;
    box.border = Qt::red;
    box.fill = QColor(0xFF, 0, 0, 48);
    ret.push_back(box);
  }

  return ret;
}




