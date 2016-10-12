#ifndef WIDGET_PLOT2D_H
#define WIDGET_PLOT2D_H

#include <QWidget>
#include "qsquarecustomplot.h"
#include "appearance.h"
#include <memory>
#include "entry2d.h"

struct MarkerBox2D {
  MarkerBox2D()
    : selected(false)
    , selectable (true)
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
  double x1, x2, y1, y2;
  QString label;
  QColor border;
  QColor fill;
};


class WidgetPlot2D : public QSquareCustomPlot
{
  Q_OBJECT

public:
  explicit WidgetPlot2D(QWidget *parent = 0);

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

  void plot_2d_mouse_clicked(double x, double y, QMouseEvent* event, bool channels);

  void optionsChanged(QAction*);
  void exportRequested(QAction*);
  void clicked_item(QCPAbstractItem*);

private:
  QCPColorMap *colorMap;

  std::map<QString, QCPAxis::ScaleType> scale_types_
  {
    {"Linear", QCPAxis::stLinear},
    {"Logarithmic", QCPAxis::stLogarithmic}
  };

  std::map<QString, QCPColorGradient> gradients_
  {
    {"Grayscale", QCPColorGradient::gpGrayscale},
    {"Hot",  QCPColorGradient::gpHot},
    {"Cold", QCPColorGradient::gpCold},
    {"Night", QCPColorGradient::gpNight},
    {"Candy", QCPColorGradient::gpCandy},
    {"Geography", QCPColorGradient::gpGeography},
    {"Ion", QCPColorGradient::gpIon},
    {"Thermal", QCPColorGradient::gpThermal},
    {"Polar", QCPColorGradient::gpPolar},
    {"Spectrum", QCPColorGradient::gpSpectrum},
    {"Jet", QCPColorGradient::gpJet},
    {"Hues", QCPColorGradient::gpHues}
  };

  QString current_gradient_;

  QMenu menuExportFormat;
  QMenu       menuOptions;
  bool show_gradient_scale_;


  QString current_scale_type_;

  QString Z_label_;

  std::list<MarkerBox2D> boxes_;


  bool show_labels_, antialiased_;


  void build_menu();
  void toggle_gradient_scale(int fontUpscale = 0);
  void plotButtons();

  void makeCustomGradients();
  void addCustomGradient(QString name, std::initializer_list<std::string> colors);

  void setColorScheme(QColor fore, QColor back, QColor grid1, QColor grid2);
};

#endif // WIDGET_PLOT2D_H
