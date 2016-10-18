#ifndef WIDGET_PLOT_MULTI1D_H
#define WIDGET_PLOT_MULTI1D_H

#include <QWidget>
#include "qp_generic.h"
#include "qp_appearance.h"

namespace QPlot
{

struct Marker1D
{
  Marker1D() {}
  Marker1D(double p, Appearance app = Appearance(), bool vis = true)
    : pos(p), visible(vis), appearance(app) {}

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

  void clearAll() override;
  void clearExtras() override;
  void replotExtras() override;

  void reset_scales();
  void tight_x();
  void rescale();

  void setTitle(QString title);

  void addGraph(const QVector<double>& x,
                const QVector<double>& y,
                Appearance appearance,
                bool fittable = false, int32_t bits = 0);
  void setYBounds(const std::map<double, double> &minima,
                  const std::map<double, double> &maxima);
  void setAxisLabels(QString x, QString y);

  void setMarkers(const std::list<Marker1D>&);
  std::set<double> selectedMarkers();
  void setHighlight(Marker1D, Marker1D);

public slots:
  void zoomOut() Q_DECL_OVERRIDE;

signals:
  void clickedLeft(double);
  void clickedRight(double);

protected slots:
  void plot_mouse_press(QMouseEvent*);
  void plot_mouse_release(QMouseEvent*);

  void plot_rezoom();

protected:
  void mouseClicked(double x, double y, QMouseEvent *event) override;

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
