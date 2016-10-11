#include "widget_plot_multi1d.h"
#include "CustomLogger.h"
#include "qt_util.h"
#include "qcp_overlay_button.h"

WidgetPlotMulti1D::WidgetPlotMulti1D(QWidget *parent) :
  QSquareCustomPlot(parent)
{
  visible_options_  =
      (ShowOptions::style | ShowOptions::scale | ShowOptions::labels | ShowOptions::thickness | ShowOptions::grid | ShowOptions::save);


  setInteraction(QCP::iSelectItems, true);
  setInteraction(QCP::iRangeDrag, true);
  yAxis->axisRect()->setRangeDrag(Qt::Horizontal);
  setInteraction(QCP::iRangeZoom, true);
  setInteraction(QCP::iMultiSelect, true);
  yAxis->setPadding(28);
  setNoAntialiasingOnDrag(true);

  connect(this, SIGNAL(mouse_clicked(double,double,QMouseEvent*,bool)), this, SLOT(plot_mouse_clicked(double,double,QMouseEvent*,bool)));
//  connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(clicked_plottable(QCPAbstractPlottable*)));
  connect(this, SIGNAL(clickedAbstractItem(QCPAbstractItem*)), this, SLOT(clicked_item(QCPAbstractItem*)));
  connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selection_changed()));
  connect(this, SIGNAL(beforeReplot()), this, SLOT(plot_rezoom()));
  connect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(plot_mouse_release(QMouseEvent*)));
  connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(plot_mouse_press(QMouseEvent*)));

  minx_zoom = 0; maxx_zoom = 0;
  minx = std::numeric_limits<double>::max();
  maxx = - std::numeric_limits<double>::max();
  miny = std::numeric_limits<double>::max();
  maxy = - std::numeric_limits<double>::max();

  force_rezoom_ = false;
  mouse_pressed_ = false;

  edge_trc1 = nullptr;
  edge_trc2 = nullptr;

  plot_style_ = "Lines";
  scale_type_ = "Logarithmic";
  yAxis->setScaleType(QCPAxis::stLogarithmic);
  grid_style_ = "Grid + subgrid";
  setColorScheme(Qt::black, Qt::white, QColor(112, 112, 112), QColor(170, 170, 170));

  xAxis->grid()->setVisible(true);
  yAxis->grid()->setVisible(true);
  xAxis->grid()->setSubGridVisible(true);
  yAxis->grid()->setSubGridVisible(true);

  marker_labels_ = true;
  thickness_ = 1;

  menuExportFormat.addAction("png");
  menuExportFormat.addAction("jpg");
  menuExportFormat.addAction("pdf");
  menuExportFormat.addAction("bmp");
  connect(&menuExportFormat, SIGNAL(triggered(QAction*)), this, SLOT(exportRequested(QAction*)));

  connect(&menuOptions, SIGNAL(triggered(QAction*)), this, SLOT(optionsChanged(QAction*)));

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
  connect(shortcut, SIGNAL(activated()), this, SLOT(zoom_out()));

  build_menu();

  replot_markers();
  //  redraw();
}

void WidgetPlotMulti1D::set_visible_options(ShowOptions options) {
  visible_options_ = options;

  setInteraction(QCP::iRangeDrag, options & ShowOptions::zoom);
  setInteraction(QCP::iRangeZoom, options & ShowOptions::zoom);

  build_menu();
  replot_markers();
}

void WidgetPlotMulti1D::build_menu() {
  menuOptions.clear();

  if (visible_options_ & ShowOptions::style) {
    menuOptions.addAction("Step center");
    menuOptions.addAction("Step left");
    menuOptions.addAction("Step right");
    menuOptions.addAction("Lines");
    menuOptions.addAction("Scatter");
    menuOptions.addAction("Fill");
  }

  if (visible_options_ & ShowOptions::scale) {
    menuOptions.addSeparator();
    menuOptions.addAction("Linear");
    menuOptions.addAction("Logarithmic");
  }

  if (visible_options_ & ShowOptions::labels) {
    menuOptions.addSeparator();
    menuOptions.addAction("Energy labels");
  }

  if (visible_options_ & ShowOptions::thickness) {
    menuOptions.addSeparator();
    menuOptions.addAction("1");
    menuOptions.addAction("2");
    menuOptions.addAction("3");
  }

  if (visible_options_ & ShowOptions::grid) {
    menuOptions.addSeparator();
    menuOptions.addAction("No grid");
    menuOptions.addAction("Grid");
    menuOptions.addAction("Grid + subgrid");
  }

  for (auto &q : menuOptions.actions()) {
    q->setCheckable(true);
    q->setChecked((q->text() == scale_type_) ||
                  (q->text() == plot_style_) ||
                  (q->text() == grid_style_) ||
                  (q->text() == QString::number(thickness_)) ||
                  ((q->text() == "Energy labels") && marker_labels_));
  }
}


void WidgetPlotMulti1D::clearGraphs()
{
  QSquareCustomPlot::clearGraphs();
  minima_.clear();
  maxima_.clear();
}

void WidgetPlotMulti1D::clearExtras()
{
  //DBG << "WidgetPlotMulti1D::clearExtras()";
  my_markers_.clear();
  rect.clear();
}

void WidgetPlotMulti1D::rescale() {
  force_rezoom_ = true;
  plot_rezoom();
}

void WidgetPlotMulti1D::redraw() {
  replot();
}

void WidgetPlotMulti1D::reset_scales()
{
  minx = std::numeric_limits<double>::max();
  maxx = - std::numeric_limits<double>::max();
  miny = std::numeric_limits<double>::max();
  maxy = - std::numeric_limits<double>::max();
  rescaleAxes();
}

void WidgetPlotMulti1D::setTitle(QString title) {
  title_text_ = title;
  replot_markers();
}

void WidgetPlotMulti1D::setLabels(QString x, QString y) {
  xAxis->setLabel(x);
  yAxis->setLabel(y);
}

void WidgetPlotMulti1D::set_markers(const std::list<Marker1D>& markers) {
  my_markers_ = markers;
}

void WidgetPlotMulti1D::set_block(Marker1D a, Marker1D b) {
  rect.resize(2);
  rect[0] = a;
  rect[1] = b;
}

std::set<double> WidgetPlotMulti1D::get_selected_markers() {
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


void WidgetPlotMulti1D::setYBounds(const std::map<double, double> &minima, const std::map<double, double> &maxima) {
  minima_ = minima;
  maxima_ = maxima;
  rescale();
}


void WidgetPlotMulti1D::addGraph(const QVector<double>& x, const QVector<double>& y, AppearanceProfile appearance, bool fittable, int32_t bits) {
  if (x.empty() || y.empty() || (x.size() != y.size()))
    return;

  QSquareCustomPlot::addGraph();
  int g = graphCount() - 1;
  graph(g)->addData(x, y);
  QPen pen = appearance.default_pen;
  if (fittable && (visible_options_ & ShowOptions::thickness))
    pen.setWidth(thickness_);
  graph(g)->setPen(pen);
  graph(g)->setProperty("fittable", fittable);
  graph(g)->setProperty("bits", QVariant::fromValue(bits));
  set_graph_style(graph(g), plot_style_);

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

void WidgetPlotMulti1D::addPoints(const QVector<double>& x, const QVector<double>& y, AppearanceProfile appearance, QCPScatterStyle::ScatterShape shape) {
  if (x.empty() || y.empty() || (x.size() != y.size()))
    return;

  QSquareCustomPlot::addGraph();
  int g = graphCount() - 1;
  graph(g)->addData(x, y);
  graph(g)->setPen(appearance.default_pen);
  graph(g)->setBrush(QBrush());
  graph(g)->setScatterStyle(QCPScatterStyle(shape, appearance.default_pen.color(), appearance.default_pen.color(), 6 /*appearance.default_pen.width()*/));
  graph(g)->setLineStyle(QCPGraph::lsNone);

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


void WidgetPlotMulti1D::plot_rezoom() {
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

void WidgetPlotMulti1D::tight_x() {
  //DBG << "tightning x to " << minx << " " << maxx;
  xAxis->setRangeLower(minx);
  xAxis->setRangeUpper(maxx);
}

void WidgetPlotMulti1D::calc_y_bounds(double lower, double upper) {
  miny = std::numeric_limits<double>::max();
  maxy = - std::numeric_limits<double>::min();

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

void WidgetPlotMulti1D::setColorScheme(QColor fore, QColor back, QColor grid1, QColor grid2)
{
  xAxis->setBasePen(QPen(fore, 1));
  yAxis->setBasePen(QPen(fore, 1));
  xAxis->setTickPen(QPen(fore, 1));
  yAxis->setTickPen(QPen(fore, 1));
  xAxis->setSubTickPen(QPen(fore, 1));
  yAxis->setSubTickPen(QPen(fore, 1));
  xAxis->setTickLabelColor(fore);
  yAxis->setTickLabelColor(fore);
  xAxis->setLabelColor(fore);
  yAxis->setLabelColor(fore);
  xAxis->grid()->setPen(QPen(grid1, 1, Qt::DotLine));
  yAxis->grid()->setPen(QPen(grid1, 1, Qt::DotLine));
  xAxis->grid()->setSubGridPen(QPen(grid2, 1, Qt::DotLine));
  yAxis->grid()->setSubGridPen(QPen(grid2, 1, Qt::DotLine));
  xAxis->grid()->setZeroLinePen(Qt::NoPen);
  yAxis->grid()->setZeroLinePen(Qt::NoPen);
  setBackground(QBrush(back));
}

void WidgetPlotMulti1D::replot_markers() {
  clearItems();
  edge_trc1 = nullptr;
  edge_trc2 = nullptr;
  double min_marker = std::numeric_limits<double>::max();
  double max_marker = - std::numeric_limits<double>::max();
  //  int total_markers = 0;

  for (auto &q : my_markers_) {
    QCPItemTracer *top_crs = nullptr;
    if (q.visible) {

      double max = std::numeric_limits<double>::lowest();
      int total = graphCount();
      for (int i=0; i < total; i++) {

        if ((graph(i)->scatterStyle().shape() != QCPScatterStyle::ssNone) &&
            (graph(i)->scatterStyle().shape() != QCPScatterStyle::ssDisc))
          continue;

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
//        addItem(crs);

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
//      addItem(line);

      if (marker_labels_) {
        QCPItemText *markerText = new QCPItemText(this);
        markerText->setProperty("true_value", top_crs->graphKey());
        markerText->setProperty("position", top_crs->property("position"));

        markerText->position->setParentAnchor(top_crs->position);
        markerText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
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
//        addItem(markerText);
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
//    addItem(cprect);
  }

  if (!title_text_.isEmpty()) {
    QCPItemText *floatingText = new QCPItemText(this);
//    addItem(floatingText);
    floatingText->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    floatingText->position->setType(QCPItemPosition::ptAxisRectRatio);
    floatingText->position->setCoords(0.5, 0); // place position at center/top of axis rect
    floatingText->setText(title_text_);
    floatingText->setFont(QFont("Helvetica", 10));
    floatingText->setSelectable(false);
    floatingText->setColor(Qt::black);
  }

  plotButtons();

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

void WidgetPlotMulti1D::plotButtons() {
  QCPOverlayButton *overlayButton;
  QCPOverlayButton *newButton;

  newButton = new QCPOverlayButton(this,
                                   QPixmap(":/icons/oxy/22/view_fullscreen.png"),
                                   "reset_scales", "Zoom out",
                                   Qt::AlignBottom | Qt::AlignRight);
  newButton->setClipToAxisRect(false);
  newButton->topLeft->setType(QCPItemPosition::ptAbsolute);
  newButton->topLeft->setCoords(5, 5);
//  addItem(newButton);
  overlayButton = newButton;

  if (!menuOptions.isEmpty()) {
    newButton = new QCPOverlayButton(this, QPixmap(":/icons/oxy/22/view_statistics.png"),
                                     "options", "Style options",
                                     Qt::AlignBottom | Qt::AlignRight);

    newButton->setClipToAxisRect(false);
    newButton->topLeft->setParentAnchor(overlayButton->bottomLeft);
    newButton->topLeft->setCoords(0, 5);
//    addItem(newButton);
    overlayButton = newButton;
  }

  if (visible_options_ & ShowOptions::save) {
    newButton = new QCPOverlayButton(this,
                                     QPixmap(":/icons/oxy/22/document_save.png"),
                                     "export", "Export plot",
                                     Qt::AlignBottom | Qt::AlignRight);
    newButton->setClipToAxisRect(false);
    newButton->topLeft->setParentAnchor(overlayButton->bottomLeft);
    newButton->topLeft->setCoords(0, 5);
//    addItem(newButton);
    overlayButton = newButton;
  }
}


void WidgetPlotMulti1D::plot_mouse_clicked(double x, double y, QMouseEvent* event, bool on_item) {
  if (event->button() == Qt::RightButton) {
    emit clickedRight(x);
  } else if (!on_item) { //tricky
    emit clickedLeft(x);
  }
}


void WidgetPlotMulti1D::selection_changed() {
  emit markers_selected();
}

//void WidgetPlotMulti1D::clicked_plottable(QCPAbstractPlottable *plt) {
//  //  LINFO << "<WidgetPlotMulti1D> clickedplottable";
//}

void WidgetPlotMulti1D::clicked_item(QCPAbstractItem* itm) {
  if (!itm->visible())
    return;

  if (QCPOverlayButton *button = qobject_cast<QCPOverlayButton*>(itm)) {
    //    QPoint p = this->mapFromGlobal(QCursor::pos());
    if (button->name() == "options") {
      menuOptions.exec(QCursor::pos());
    } else if (button->name() == "export") {
      menuExportFormat.exec(QCursor::pos());
    } else if (button->name() == "reset_scales") {
      zoom_out();
    }
  }
}

void WidgetPlotMulti1D::zoom_out()
{
  xAxis->rescale();
  force_rezoom_ = true;
  plot_rezoom();
  replot();
}

void WidgetPlotMulti1D::plot_mouse_press(QMouseEvent*) {
  disconnect(this, 0, this, 0);
  connect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(plot_mouse_release(QMouseEvent*)));
  connect(this, SIGNAL(clickedAbstractItem(QCPAbstractItem*)), this, SLOT(clicked_item(QCPAbstractItem*)));
  connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selection_changed()));

  force_rezoom_ = false;
  mouse_pressed_ = true;
}

void WidgetPlotMulti1D::plot_mouse_release(QMouseEvent*) {
  connect(this, SIGNAL(mouse_clicked(double,double,QMouseEvent*,bool)), this, SLOT(plot_mouse_clicked(double,double,QMouseEvent*,bool)));
  connect(this, SIGNAL(beforeReplot()), this, SLOT(plot_rezoom()));
  connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(plot_mouse_press(QMouseEvent*)));
  force_rezoom_ = true;
  mouse_pressed_ = false;
  plot_rezoom();
  replot();
}

void WidgetPlotMulti1D::optionsChanged(QAction* action) {
  this->setCursor(Qt::WaitCursor);
  QString choice = action->text();
  if (choice == "Linear") {
    scale_type_ = choice;
    yAxis->setScaleType(QCPAxis::stLinear);
  } else if (choice == "Logarithmic") {
    yAxis->setScaleType(QCPAxis::stLogarithmic);
    scale_type_ = choice;
  } else if ((choice == "Scatter") || (choice == "Lines") || (choice == "Fill")
             || (choice == "Step center") || (choice == "Step left") || (choice == "Step right")) {
    plot_style_ = choice;
    int total = graphCount();
    for (int i=0; i < total; i++)
      if ((graph(i)->scatterStyle().shape() == QCPScatterStyle::ssNone) || (graph(i)->scatterStyle().shape() == QCPScatterStyle::ssDisc))
        set_graph_style(graph(i), choice);
  } else if (choice == "Energy labels") {
    marker_labels_ = !marker_labels_;
    replot_markers();
  } else if (choice == "1") {
    thickness_ = 1;
    int total = graphCount();
    for (int i=0; i < total; i++)
      if ((graph(i)->scatterStyle().shape() == QCPScatterStyle::ssNone) || (graph(i)->scatterStyle().shape() == QCPScatterStyle::ssDisc))
        set_graph_style(graph(i), choice);
  } else if (choice == "2") {
    thickness_ = 2;
    int total = graphCount();
    for (int i=0; i < total; i++)
      if ((graph(i)->scatterStyle().shape() == QCPScatterStyle::ssNone) || (graph(i)->scatterStyle().shape() == QCPScatterStyle::ssDisc))
        set_graph_style(graph(i), choice);
  } else if (choice == "3") {
    thickness_ = 3;
    int total = graphCount();
    for (int i=0; i < total; i++)
      if ((graph(i)->scatterStyle().shape() == QCPScatterStyle::ssNone) || (graph(i)->scatterStyle().shape() == QCPScatterStyle::ssDisc))
        set_graph_style(graph(i), choice);
  } else if ((choice == "No grid") || (choice == "Grid") || (choice == "Grid + subgrid")) {
    grid_style_ = choice;
    xAxis->grid()->setVisible(grid_style_ != "No grid");
    yAxis->grid()->setVisible(grid_style_ != "No grid");
    xAxis->grid()->setSubGridVisible(grid_style_ == "Grid + subgrid");
    yAxis->grid()->setSubGridVisible(grid_style_ == "Grid + subgrid");
  }

  build_menu();
  replot();
  this->setCursor(Qt::ArrowCursor);
}

void WidgetPlotMulti1D::set_grid_style(QString grd) {
  if ((grd == "No grid") || (grd == "Grid") || (grd == "Grid + subgrid")) {
    grid_style_ = grd;
    xAxis->grid()->setVisible(grid_style_ != "No grid");
    yAxis->grid()->setVisible(grid_style_ != "No grid");
    xAxis->grid()->setSubGridVisible(grid_style_ == "Grid + subgrid");
    yAxis->grid()->setSubGridVisible(grid_style_ == "Grid + subgrid");
    build_menu();
  }
}

QString WidgetPlotMulti1D::scale_type() {
  return scale_type_;
}

QString WidgetPlotMulti1D::plot_style() {
  return plot_style_;
}

QString WidgetPlotMulti1D::grid_style() {
  return grid_style_;
}

void WidgetPlotMulti1D::set_graph_style(QCPGraph* graph, QString style) {
  if (!graph->property("fittable").toBool()) {
    graph->setBrush(QBrush());
    graph->setLineStyle(QCPGraph::lsLine);
    graph->setScatterStyle(QCPScatterStyle::ssNone);
  } else {
    if (style == "Fill") {
      graph->setBrush(QBrush(graph->pen().color()));
      graph->setLineStyle(QCPGraph::lsLine);
      graph->setScatterStyle(QCPScatterStyle::ssNone);
    } else if (style == "Lines") {
      graph->setBrush(QBrush());
      graph->setLineStyle(QCPGraph::lsLine);
      graph->setScatterStyle(QCPScatterStyle::ssNone);
    } else if (style == "Step center") {
      graph->setBrush(QBrush());
      graph->setLineStyle(QCPGraph::lsStepCenter);
      graph->setScatterStyle(QCPScatterStyle::ssNone);
    } else if (style == "Step left") {
      graph->setBrush(QBrush());
      graph->setLineStyle(QCPGraph::lsStepLeft);
      graph->setScatterStyle(QCPScatterStyle::ssNone);
    } else if (style == "Step right") {
      graph->setBrush(QBrush());
      graph->setLineStyle(QCPGraph::lsStepRight);
      graph->setScatterStyle(QCPScatterStyle::ssNone);
    } else if (style == "Scatter") {
      graph->setBrush(QBrush());
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc));
      graph->setLineStyle(QCPGraph::lsNone);
    }

    if (visible_options_ & ShowOptions::thickness) {
      QPen pen = graph->pen();
      pen.setWidth(thickness_);
      graph->setPen(pen);
    }
  }

}

void WidgetPlotMulti1D::set_scale_type(QString sct) {
  this->setCursor(Qt::WaitCursor);
  scale_type_ = sct;
  if (scale_type_ == "Linear")
    yAxis->setScaleType(QCPAxis::stLinear);
  else if (scale_type() == "Logarithmic")
    yAxis->setScaleType(QCPAxis::stLogarithmic);
  replot();
  build_menu();
  this->setCursor(Qt::ArrowCursor);
}

void WidgetPlotMulti1D::set_plot_style(QString stl) {
  this->setCursor(Qt::WaitCursor);
  plot_style_ = stl;
  int total = graphCount();
  for (int i=0; i < total; i++) {
    if ((graph(i)->scatterStyle().shape() == QCPScatterStyle::ssNone) || (graph(i)->scatterStyle().shape() == QCPScatterStyle::ssDisc))
      set_graph_style(graph(i), stl);
  }
  build_menu();
  replot();
  this->setCursor(Qt::ArrowCursor);
}

void WidgetPlotMulti1D::set_marker_labels(bool sl)
{
  marker_labels_ = sl;
  replot_markers();
  build_menu();
  replot();
}

bool WidgetPlotMulti1D::marker_labels() {
  return marker_labels_;
}


void WidgetPlotMulti1D::exportRequested(QAction* choice) {
  QString filter = choice->text() + "(*." + choice->text() + ")";
  QString fileName = CustomSaveFileDialog(this, "Export plot",
                                          QStandardPaths::locate(QStandardPaths::HomeLocation, ""),
                                          filter);
  if (validateFile(this, fileName, true)) {

    int fontUpscale = 5;

    for (int i = 0; i < itemCount(); ++i) {
      QCPAbstractItem* itm = item(i);
      if (QCPItemLine *line = qobject_cast<QCPItemLine*>(itm))
      {
        QCPLineEnding head = line->head();
        QPen pen = line->selectedPen();
        head.setWidth(head.width() + fontUpscale);
        head.setLength(head.length() + fontUpscale);
        line->setHead(head);
        line->setPen(pen);
        line->start->setCoords(0, -50);
        line->end->setCoords(0, -15);
      }
      else if (QCPItemText *txt = qobject_cast<QCPItemText*>(itm))
      {
        QPen pen = txt->selectedPen();
        txt->setPen(pen);
        QFont font = txt->font();
        font.setPointSize(font.pointSize() + fontUpscale);
        txt->setFont(font);
        txt->setColor(pen.color());
        txt->position->setCoords(0, -50);
      }
    }

    prepPlotExport(2, fontUpscale, 20);
    replot();

    plot_rezoom();
    for (int i = 0; i < itemCount(); ++i) {
      QCPAbstractItem* itm = item(i);
      if (QCPOverlayButton *btn = qobject_cast<QCPOverlayButton*>(itm))
        btn->setVisible(false);
    }

    replot();

    QFileInfo file(fileName);
    if (file.suffix() == "png")
      savePng(fileName,0,0,1,100);
    else if (file.suffix() == "jpg")
      saveJpg(fileName,0,0,1,100);
    else if (file.suffix() == "bmp")
      saveBmp(fileName);
    else if (file.suffix() == "pdf")
      savePdf(fileName, true);


    for (int i = 0; i < itemCount(); ++i) {
      QCPAbstractItem* itm = item(i);
      if (QCPItemLine *line = qobject_cast<QCPItemLine*>(itm))
      {
        QCPLineEnding head = line->head();
        QPen pen = line->selectedPen();
        head.setWidth(head.width() - fontUpscale);
        head.setLength(head.length() - fontUpscale);
        line->setHead(head);
        line->setPen(pen);
        line->start->setCoords(0, -30);
        line->end->setCoords(0, -5);
      }
      else if (QCPItemText *txt = qobject_cast<QCPItemText*>(itm))
      {
        QPen pen = txt->selectedPen();
        txt->setPen(pen);
        QFont font = txt->font();
        font.setPointSize(font.pointSize() - fontUpscale);
        txt->setFont(font);
        txt->setColor(pen.color());
        txt->position->setCoords(0, -30);
      }
    }

    postPlotExport(2, fontUpscale, 20);
    replot();
    plot_rezoom();
    for (int i = 0; i < itemCount(); ++i) {
      QCPAbstractItem* itm = item(i);
      if (QCPOverlayButton *btn = qobject_cast<QCPOverlayButton*>(itm))
        btn->setVisible(true);
    }

    replot();



  }
}
