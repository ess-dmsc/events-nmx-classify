#ifndef WIDGET_PLOT2D_H
#define WIDGET_PLOT2D_H

#include <QWidget>
#include "qp_generic.h"
#include "qp_appearance.h"
#include <memory>
#include "qp_entry2d.h"

namespace QPlot
{

struct MarkerBox2D
{
  bool operator== (const MarkerBox2D& other) const
  {
    if (x1 != other.x1) return false;
    if (x2 != other.x2) return false;
    if (y1 != other.y1) return false;
    if (y2 != other.y2) return false;
    return true;
  }

  bool operator!= (const MarkerBox2D& other) const
    { return !operator==(other); }

  bool selected {false};
  bool selectable {false};
  double x1, x2, y1, y2;
  QString label;
  QColor border;
  QColor fill;
};


class Plot2D : public GenericPlot
{
  Q_OBJECT

public:
  explicit Plot2D(QWidget *parent = 0);

  void update_plot(uint64_t sizex, uint64_t sizey, const EntryList &spectrum_data);
  void set_axes(QString xlabel, double x1, double x2,
                QString ylabel, double y1, double y2,
                QString zlabel);
  void set_zoom_drag(Qt::Orientation);
  void refresh();
  void replot_markers();
  void reset_content();

  void set_boxes(std::list<MarkerBox2D> boxes);

  std::list<MarkerBox2D> get_selected_boxes();

public slots:
  void zoom_out();

signals:
  void markers_set(double x, double y, bool left_mouse);
  void stuff_selected();

private slots:
  void selection_changed();
  void plot_2d_mouse_clicked(double x, double y, QMouseEvent* event, bool channels);
  void clicked_item(QCPAbstractItem*);

private:
  QCPColorMap *colorMap {new QCPColorMap(xAxis, yAxis)};
  std::list<MarkerBox2D> boxes_;
  QString Z_label_;

  void initializeGradients();
  void addCustomGradient(QString name, std::initializer_list<std::string> colors);
};

}

#endif
