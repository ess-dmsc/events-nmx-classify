#ifndef WIDGET_PLOT_MULTI1D_H
#define WIDGET_PLOT_MULTI1D_H

#include <QWidget>
#include "qp_generic.h"
#include <set>
#include "qp_appearance.h"

namespace QPlot
{

struct Marker1D
{
  bool operator!= (const Marker1D& other) const { return (!operator==(other)); }
  bool operator== (const Marker1D& other) const { return (pos == other.pos); }

  bool visible {false};
  double pos {0};
  Appearance appearance;
};


class Multi1D : public GenericPlot
{
  Q_OBJECT

public:
  explicit Multi1D(QWidget *parent = 0);
  virtual void clearGraphs();

  void clearExtras();

  void redraw();
  void reset_scales();

  void tight_x();
  void rescale();
  void replot_markers();

  void setLabels(QString x, QString y);
  void setTitle(QString title);

  void set_visible_options(ShowOptions);

  void set_markers(const std::list<Marker1D>&);
  void set_block(Marker1D, Marker1D);

  std::set<double> get_selected_markers();

  void addGraph(const QVector<double>& x,
                const QVector<double>& y,
                Appearance appearance,
                bool fittable = false, int32_t bits = 0);

  void setYBounds(const std::map<double, double> &minima,
                  const std::map<double, double> &maxima);

public slots:
  void zoom_out();

signals:
  void clickedLeft(double);
  void clickedRight(double);
  void markers_selected();

protected slots:
  void plot_mouse_clicked(double x, double y, QMouseEvent *event, bool channels);
  void plot_mouse_press(QMouseEvent*);
  void plot_mouse_release(QMouseEvent*);
  void clicked_item(QCPAbstractItem *);

  void selection_changed();
  void plot_rezoom();

protected:
  QString title_text_;
  std::list<Marker1D> my_markers_;
  std::vector<Marker1D> rect;

  bool force_rezoom_ {false};
  bool mouse_pressed_ {false};

  std::map<double, double> minima_, maxima_;
  double minx {std::numeric_limits<double>::max()};
  double maxx {std::numeric_limits<double>::min()};
  double miny {std::numeric_limits<double>::max()};
  double maxy {std::numeric_limits<double>::min()};
  double minx_zoom {0};
  double maxx_zoom {0};

  void calc_y_bounds(double lower, double upper);
};

}

#endif
