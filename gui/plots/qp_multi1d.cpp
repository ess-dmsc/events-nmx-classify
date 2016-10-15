#include "qp_multi1d.h"
#include "CustomLogger.h"
#include "qt_util.h"
#include "qp_button.h"

namespace QPlot
{

Multi1D::Multi1D(QWidget *parent)
  : GenericPlot(parent)
{
  visible_options_ =
      (ShowOptions::zoom | ShowOptions::save |
       ShowOptions::scale | ShowOptions::style | ShowOptions::thickness);

  setInteraction(QCP::iSelectItems, true);
  setInteraction(QCP::iRangeDrag, true);
  yAxis->axisRect()->setRangeDrag(Qt::Horizontal);
  setInteraction(QCP::iRangeZoom, true);
  setInteraction(QCP::iMultiSelect, true);
  yAxis->setPadding(28);
  setNoAntialiasingOnDrag(true);

  connect(this, SIGNAL(mouseClicked(double,double,QMouseEvent*,bool)), this, SLOT(plot_mouse_clicked(double,double,QMouseEvent*,bool)));
  //  connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(clicked_plottable(QCPAbstractPlottable*)));
  connect(this, SIGNAL(clickedAbstractItem(QCPAbstractItem*)), this, SLOT(clicked_item(QCPAbstractItem*)));
  connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selection_changed()));
  connect(this, SIGNAL(beforeReplot()), this, SLOT(plot_rezoom()));
  connect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(plot_mouse_release(QMouseEvent*)));
  connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(plot_mouse_press(QMouseEvent*)));

  setScaleType("Logarithmic");
  setPlotStyle("Step center");
  setGridStyle(current_grid_style_);
  setColorScheme(Qt::black, Qt::white, QColor(112, 112, 112), QColor(170, 170, 170));

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
  connect(shortcut, SIGNAL(activated()), this, SLOT(zoom_out()));

  rebuild_menu();
  replot_markers();
}

void Multi1D::set_visible_options(ShowOptions options) {
  visible_options_ = options;

  setInteraction(QCP::iRangeDrag, options & ShowOptions::zoom);
  setInteraction(QCP::iRangeZoom, options & ShowOptions::zoom);

  rebuild_menu();
  replot_markers();
}

void Multi1D::clearGraphs()
{
  GenericPlot::clearGraphs();
  minima_.clear();
  maxima_.clear();
}

void Multi1D::clearExtras()
{
  //DBG << "Multi1D::clearExtras()";
  my_markers_.clear();
  rect.clear();
}

void Multi1D::rescale() {
  force_rezoom_ = true;
  plot_rezoom();
}

void Multi1D::redraw() {
  replot();
}

void Multi1D::reset_scales()
{
  minx = std::numeric_limits<double>::max();
  maxx = std::numeric_limits<double>::min();
  miny = std::numeric_limits<double>::max();
  maxy = std::numeric_limits<double>::min();
  rescaleAxes();
}

void Multi1D::setTitle(QString title) {
  title_text_ = title;
  replot_markers();
}

void Multi1D::setLabels(QString x, QString y) {
  xAxis->setLabel(x);
  yAxis->setLabel(y);
}

void Multi1D::set_markers(const std::list<Marker1D>& markers) {
  my_markers_ = markers;
}

void Multi1D::set_block(Marker1D a, Marker1D b) {
  rect.resize(2);
  rect[0] = a;
  rect[1] = b;
}

std::set<double> Multi1D::get_selected_markers() {
  std::set<double> selection;
  for (auto &q : selectedItems())
    if (QCPItemText *txt = qobject_cast<QCPItemText*>(q)) {
      if (txt->property("position").isValid())
        selection.insert(txt->property("position").toDouble());
      //DBG << "found selected " << txt->property("true_value").toDouble() << " chan=" << txt->property("position").toDouble();
    } else if (QCPItemLine *line = qobject_cast<QCPItemLine*>(q)) {
      if (line->property("position").isValid())
        selection.insert(line->property("position").toDouble());
      //DBG << "found selected " << line->property("true_value").toDouble() << " chan=" << line->property("position").toDouble();
    }

  return selection;
}


void Multi1D::setYBounds(const std::map<double, double> &minima,
                         const std::map<double, double> &maxima)
{
  minima_ = minima;
  maxima_ = maxima;
  rescale();
}


void Multi1D::addGraph(const QVector<double>& x, const QVector<double>& y,
                       Appearance appearance, bool fittable, int32_t bits)
{
  if (x.empty() || y.empty() || (x.size() != y.size()))
    return;

  GenericPlot::addGraph();
  int g = graphCount() - 1;
  graph(g)->addData(x, y);
  graph(g)->setPen(appearance.default_pen);
  graph(g)->setProperty("fittable", fittable);
  graph(g)->setProperty("bits", QVariant::fromValue(bits));
  set_graph_style(graph(g), current_plotStyle_);

  if (x[0] < minx) {
    minx = x[0];
    //DBG << "new minx " << minx;
    xAxis->rescale();
  }
  if (x[x.size() - 1] > maxx) {
    maxx = x[x.size() - 1];
    xAxis->rescale();
  }
}

void Multi1D::plot_rezoom() {
  if (mouse_pressed_)
    return;

  if (minima_.empty() || maxima_.empty()) {
    yAxis->rescale();
    return;
  }

  double upperc = xAxis->range().upper;
  double lowerc = xAxis->range().lower;

  if (!force_rezoom_ && (lowerc == minx_zoom) && (upperc == maxx_zoom))
    return;

  minx_zoom = lowerc;
  maxx_zoom = upperc;
  force_rezoom_ = false;

  calc_y_bounds(lowerc, upperc);

  //DBG << "Rezoom";

  if (miny <= 0)
    yAxis->rescale();
  else
    yAxis->setRangeLower(miny);
  yAxis->setRangeUpper(maxy);
}

void Multi1D::tight_x() {
  //DBG << "tightning x to " << minx << " " << maxx;
  xAxis->setRangeLower(minx);
  xAxis->setRangeUpper(maxx);
}

void Multi1D::calc_y_bounds(double lower, double upper) {
  miny = std::numeric_limits<double>::max();
  maxy = std::numeric_limits<double>::min();

  for (std::map<double, double>::const_iterator it = minima_.lower_bound(lower); it != minima_.upper_bound(upper); ++it)
    if (it->second < miny)
      miny = it->second;

  for (std::map<double, double>::const_iterator it = maxima_.lower_bound(lower); it != maxima_.upper_bound(upper); ++it) {
    if (it->second > maxy)
      maxy = it->second;
  }

  maxy = yAxis->pixelToCoord(yAxis->coordToPixel(maxy) - 75);

  /*if ((maxy > 1) && (miny == 0))
    miny = 1;*/
}

void Multi1D::replot_markers() {
  clearItems();
  double min_marker = std::numeric_limits<double>::max();
  double max_marker = std::numeric_limits<double>::min();
  //  int total_markers = 0;

  for (auto &q : my_markers_) {
    QCPItemTracer *top_crs = nullptr;
    if (q.visible) {

      double max = std::numeric_limits<double>::lowest();
      int total = graphCount();
      for (int i=0; i < total; i++) {

        if (!graph(i)->property("fittable").toBool())
          continue;

        if ((graph(i)->data()->begin()->key >= q.pos)
            || (q.pos >= graph(i)->data()->end()->key))
          continue;

        QCPItemTracer *crs = new QCPItemTracer(this);
        crs->setStyle(QCPItemTracer::tsNone); //tsCirlce?
        crs->setProperty("position", q.pos);

        crs->setSize(4);
        crs->setGraph(graph(i));
        crs->setInterpolating(true);
        crs->setGraphKey(q.pos);
        crs->setPen(q.appearance.default_pen);
        crs->setSelectable(false);

        crs->updatePosition();
        double val = crs->positions().first()->value();
        if (val > max) {
          max = val;
          top_crs = crs;
        }
      }
    }
    if (top_crs != nullptr) {
      QPen pen = q.appearance.default_pen;

      QCPItemLine *line = new QCPItemLine(this);
      line->start->setParentAnchor(top_crs->position);
      line->start->setCoords(0, -30);
      line->end->setParentAnchor(top_crs->position);
      line->end->setCoords(0, -5);
      line->setHead(QCPLineEnding(QCPLineEnding::esLineArrow, 7, 7));
      line->setPen(pen);
      line->setSelectedPen(pen);
      line->setProperty("true_value", top_crs->graphKey());
      line->setProperty("position", top_crs->property("position"));
      line->setSelectable(false);

      if (show_marker_labels_)
      {
        QCPItemText *markerText = new QCPItemText(this);
        markerText->setProperty("true_value", top_crs->graphKey());
        markerText->setProperty("position", top_crs->property("position"));

        markerText->position->setParentAnchor(top_crs->position);
        markerText->setPositionAlignment(static_cast<Qt::AlignmentFlag>(Qt::AlignHCenter|Qt::AlignBottom));
        markerText->position->setCoords(0, -30);
        markerText->setText(QString::number(q.pos));
        markerText->setTextAlignment(Qt::AlignLeft);
        markerText->setFont(QFont("Helvetica", 9));
        markerText->setPen(pen);
        markerText->setColor(pen.color());
        markerText->setSelectedColor(pen.color());
        markerText->setSelectedPen(pen);
        markerText->setPadding(QMargins(1, 1, 1, 1));
        markerText->setSelectable(false);
      }
    }

    //xAxis->setRangeLower(min_marker);
    //xAxis->setRangeUpper(max_marker);
    //xAxis->setRange(min, max);
    //xAxis2->setRange(min, max);

  }

  if ((rect.size() == 2) && (rect[0].visible) && !maxima_.empty() && !minima_.empty()){
    double upperc = maxima_.rbegin()->first;
    double lowerc = maxima_.begin()->first;

    calc_y_bounds(lowerc, upperc);

    double pos1 = rect[0].pos;
    double pos2 = rect[1].pos;

    QCPItemRect *cprect = new QCPItemRect(this);
    double x1 = pos1;
    double y1 = maxy;
    double x2 = pos2;
    double y2 = miny;

    //DBG << "will make box x=" << x1 << "-" << x2 << " y=" << y1 << "-" << y2;

    cprect->topLeft->setCoords(x1, y1);
    cprect->bottomRight->setCoords(x2, y2);
    cprect->setPen(rect[0].appearance.default_pen);
    cprect->setBrush(QBrush(rect[1].appearance.default_pen.color()));
    cprect->setSelectable(false);
  }

  if (!title_text_.isEmpty()) {
    QCPItemText *floatingText = new QCPItemText(this);
    floatingText->setPositionAlignment(static_cast<Qt::AlignmentFlag>(Qt::AlignTop|Qt::AlignHCenter));
    floatingText->position->setType(QCPItemPosition::ptAxisRectRatio);
    floatingText->position->setCoords(0.5, 0); // place position at center/top of axis rect
    floatingText->setText(title_text_);
    floatingText->setFont(QFont("Helvetica", 10));
    floatingText->setSelectable(false);
    floatingText->setColor(Qt::black);
  }

  plot_buttons();

  bool xaxis_changed = false;
  double dif_lower = min_marker - xAxis->range().lower;
  double dif_upper = max_marker - xAxis->range().upper;
  if (dif_upper > 0) {
    xAxis->setRangeUpper(max_marker + 20);
    if (dif_lower > (dif_upper + 20))
      xAxis->setRangeLower(xAxis->range().lower + dif_upper + 20);
    xaxis_changed = true;
  }

  if (dif_lower < 0) {
    xAxis->setRangeLower(min_marker - 20);
    if (dif_upper < (dif_lower - 20))
      xAxis->setRangeUpper(xAxis->range().upper + dif_lower - 20);
    xaxis_changed = true;
  }

  if (xaxis_changed) {
    replot();
    plot_rezoom();
  }

}

void Multi1D::plot_mouse_clicked(double x, double y, QMouseEvent* event, bool on_item) {
  if (event->button() == Qt::RightButton) {
    emit clickedRight(x);
  } else if (!on_item) { //tricky
    emit clickedLeft(x);
  }
}


void Multi1D::selection_changed() {
  emit markers_selected();
}

//void Multi1D::clicked_plottable(QCPAbstractPlottable *plt) {
//  //  LINFO << "<Multi1D> clickedplottable";
//}

void Multi1D::clicked_item(QCPAbstractItem* itm) {
  if (!itm->visible())
    return;

  if (Button *button = qobject_cast<Button*>(itm))
  {
    //    QPoint p = this->mapFromGlobal(QCursor::pos());
    if (button->name() == "options") {
      options_menu_.exec(QCursor::pos());
    } else if (button->name() == "export") {
      export_menu_.exec(QCursor::pos());
    } else if (button->name() == "reset_scales") {
      zoom_out();
    }
  }
}

void Multi1D::zoom_out()
{
  xAxis->rescale();
  force_rezoom_ = true;
  plot_rezoom();
  replot();
}

void Multi1D::plot_mouse_press(QMouseEvent*) {
  disconnect(this, 0, this, 0);
  connect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(plot_mouse_release(QMouseEvent*)));
  connect(this, SIGNAL(clickedAbstractItem(QCPAbstractItem*)), this, SLOT(clicked_item(QCPAbstractItem*)));
  connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selection_changed()));

  force_rezoom_ = false;
  mouse_pressed_ = true;
}

void Multi1D::plot_mouse_release(QMouseEvent*) {
  connect(this, SIGNAL(mouse_clicked(double,double,QMouseEvent*,bool)), this, SLOT(plot_mouse_clicked(double,double,QMouseEvent*,bool)));
  connect(this, SIGNAL(beforeReplot()), this, SLOT(plot_rezoom()));
  connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(plot_mouse_press(QMouseEvent*)));
  force_rezoom_ = true;
  mouse_pressed_ = false;
  plot_rezoom();
  replot();
}


}
