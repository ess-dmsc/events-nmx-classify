#ifndef WIDGET_PLOT2D_H
#define WIDGET_PLOT2D_H

#include <QWidget>
#include "qsquarecustomplot.h"
#include "appearance.h"
#include <memory>
#include "entry2d.h"

namespace Ui {
class WidgetPlot2D;
}

struct MarkerBox2D {
  MarkerBox2D()
    : selected(false)
    , selectable (true)
    , mark_center(false)
  {}

  bool operator== (const MarkerBox2D& other) const {
    if (x1 != other.x1) return false;
    if (x2 != other.x2) return false;
    if (y1 != other.y1) return false;
    if (y2 != other.y2) return false;
    return true;
  }

  bool operator!= (const MarkerBox2D& other) const
    { return !operator==(other); }

  bool selected;
  bool selectable;
  bool mark_center;
  double x1, x2, y1, y2;
  QString label;
};


class WidgetPlot2D : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetPlot2D(QWidget *parent = 0);
  ~WidgetPlot2D();

  void update_plot(uint64_t sizex, uint64_t sizey, const EntryList &spectrum_data);
  void set_axes(QString xlabel, double x1, double x2,
                QString ylabel, double y1, double y2,
                QString zlabel);
  void set_zoom_drag(Qt::Orientation);
  void refresh();
  void replot_markers();
  void reset_content();

  void set_boxes(std::list<MarkerBox2D> boxes);

  void set_scale_type(QString);
  void set_gradient(QString);
  void set_zoom(double);
  void set_show_legend(bool);
  void set_antialiased(bool);
  QString scale_type();
  QString gradient();
  bool show_legend();

  std::list<MarkerBox2D> get_selected_boxes();

public slots:
  void zoom_out();

signals:
  void markers_set(double x, double y, bool left_mouse);
  void stuff_selected();

private slots:
  //void clicked_plottable(QCPAbstractPlottable*);
  void selection_changed();

  void plot_2d_mouse_upon(double x, double y);
  void plot_2d_mouse_clicked(double x, double y, QMouseEvent* event, bool channels);

  void optionsChanged(QAction*);
  void exportRequested(QAction*);
  void clicked_item(QCPAbstractItem*);

private:

  //gui stuff
  Ui::WidgetPlot2D *ui;
  QCPColorMap *colorMap;

  std::map<QString, QCPColorGradient> gradients_;
  QString current_gradient_;

  QMenu menuExportFormat;
  QMenu       menuOptions;
  bool show_gradient_scale_;

  std::map<QString, QCPAxis::ScaleType> scale_types_;
  QString current_scale_type_;

  QString Z_label_;

  std::list<MarkerBox2D> boxes_;

  AppearanceProfile marker_looks;

  bool show_labels_, antialiased_;


  void build_menu();
  void toggle_gradient_scale();

  void setColorScheme(QColor fore, QColor back, QColor grid1, QColor grid2);
};

#endif // WIDGET_PLOT2D_H
