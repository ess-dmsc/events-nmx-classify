#ifndef Q_SQUARE_CP_H
#define Q_SQUARE_CP_H

#include "qcustomplot.h"
#include "draggable_tracer.h"
#include <QWidget>
#include <set>

enum ShowOptions {
  empty     = 0,
  style     = 1 << 0,
  scale     = 1 << 1,
  labels    = 1 << 2,
  themes    = 1 << 3,
  thickness = 1 << 4,
  grid      = 1 << 5,
  title     = 1 << 6,
  zoom      = 1 << 7,
  save      = 1 << 8,
  gradients = 1 << 9,
  dither    = 1 << 10
};

inline ShowOptions operator|(ShowOptions a, ShowOptions b) {return static_cast<ShowOptions>(static_cast<int>(a) | static_cast<int>(b));}
inline ShowOptions operator&(ShowOptions a, ShowOptions b) {return static_cast<ShowOptions>(static_cast<int>(a) & static_cast<int>(b));}

class QSquareCustomPlot : public QCustomPlot
{
  Q_OBJECT
public:
  explicit QSquareCustomPlot(QWidget *parent = 0);

  void setAlwaysSquare(bool sq);

  QString scale_type();
  void set_scale_type(QString);

  bool marker_labels();
  void set_marker_labels(bool);

  QString grid_style();
  void set_grid_style(QString);

  uint16_t line_thickness();
  void set_line_thickness(uint16_t);

  void set_gradient(QString);
  QString gradient();

  void set_show_legend(bool);
  bool show_legend();

  void set_antialiased(bool);
  bool antialiased();

  void set_plot_style(QString);
  QString plot_style();


  QSize sizeHint() const Q_DECL_OVERRIDE;

signals:
  void mouse_upon(double x, double y);
  void mouse_clicked(double x, double y, QMouseEvent* e, bool on_item); //why on_item?
  void shiftStateChanged(bool);
  void clickedAbstractItem(QCPAbstractItem *);

protected:
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent * event) Q_DECL_OVERRIDE;

  void keyPressEvent(QKeyEvent*) Q_DECL_OVERRIDE;
  void keyReleaseEvent(QKeyEvent*) Q_DECL_OVERRIDE;

  ShowOptions visible_options_;
  QMenu options_menu_;
  QMenu export_menu_;

  QString current_scale_type_ {"Linear"};
  std::map<QString, QCPAxis::ScaleType> scale_types_
  {
    {"Linear", QCPAxis::stLinear},
    {"Logarithmic", QCPAxis::stLogarithmic}
  };

  QString current_grid_style_ {"Grid + subgrid"};
  std::set<QString> grid_styles_ {"No grid", "Grid", "Grid + subgrid"};

  QString current_plot_style_ {"Step center"};
  std::set<QString> plot_styles_ {"Step center", "Step left", "Step right",
                                  "Lines", "Scatter", "Fill"};

  QString current_gradient_;
  std::map<QString, QCPColorGradient> gradients_;

  bool show_gradient_scale_ {false};
  bool antialiased_ {false};
  bool show_marker_labels_ {true};
  uint16_t line_thickness_ {1};


  void plotButtons();
  void setColorScheme(QColor fore, QColor back, QColor grid1, QColor grid2);
  QFont rescaleFont(QFont font, double size_offset);
  void rescaleEverything(int fontUpscale, int plotThicken, int marginUpscale, bool buttons_visible);
  void set_graph_thickness(QCPGraph* graph);
  void set_graph_style(QCPGraph*, QString);
  void build_menu();

protected slots:
  void exportPlot(QAction*);
  void optionsChanged(QAction*);

private:
  mutable int lastHeight;
  bool square;
  DraggableTracer *under_mouse_;

};

#endif
