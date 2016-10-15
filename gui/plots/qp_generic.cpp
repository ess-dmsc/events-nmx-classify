#include "qp_generic.h"
#include "CustomLogger.h"
#include "qp_button.h"
#include "qt_util.h"

namespace QPlot
{

GenericPlot::GenericPlot(QWidget *parent)
  : QCustomPlot(parent)
{
  export_menu_.addAction("png");
  export_menu_.addAction("jpg");
  export_menu_.addAction("pdf");
  export_menu_.addAction("bmp");
  connect(&export_menu_, SIGNAL(triggered(QAction*)), this, SLOT(exportPlot(QAction*)));
  connect(&options_menu_, SIGNAL(triggered(QAction*)), this, SLOT(optionsChanged(QAction*)));
}


//getters

bool GenericPlot::alwaysSquare() const
{
  return always_square_;
}

bool GenericPlot::antialiased() const
{
  return antialiased_;
}

bool GenericPlot::showGradientLegend() const
{
  return show_gradient_legend_;
}

bool GenericPlot::showMarkerLabels() const
{
  return show_marker_labels_;
}

uint16_t GenericPlot::lineThickness() const
{
  return line_thickness_;
}

QString GenericPlot::scaleType() const
{
  return current_scale_type_;
}

QString GenericPlot::gridStyle() const
{
  return current_grid_style_;
}

QString GenericPlot::gradient() const
{
  return current_gradient_;
}

QString GenericPlot::plotStyle() const
{
  return current_plotStyle_;
}


//setters

void GenericPlot::setAlwaysSquare(bool sq)
{
  always_square_ = sq;
  updateGeometry();
}

void GenericPlot::setAntialiased(bool anti)
{
  antialiased_ = anti;
  for (int i=0; i < plottableCount(); ++i)
    if (QCPColorMap *colorMap = qobject_cast<QCPColorMap*>(plottable(i)))
      colorMap->setInterpolate(antialiased_);
  rebuild_menu();
}

void GenericPlot::setShowGradientLegend(bool show)
{
  if (show_gradient_legend_ == show)
    return;

  show_gradient_legend_ = show;

  QCPColorMap *colorMap {nullptr};
  for (int i=0; i < plottableCount(); ++i)
    if (colorMap = qobject_cast<QCPColorMap*>(plottable(i)))
      break;

  if (show_gradient_legend_ && colorMap)
  {
    addGradientLegend(colorMap);
    setScaleType(current_scale_type_);
  }

  updateGeometry();
  rebuild_menu();
}

void GenericPlot::removeGradientLegend()
{
  for (int i=0; i < plotLayout()->elementCount(); i++) {
    if (QCPColorScale *le = qobject_cast<QCPColorScale*>(plotLayout()->elementAt(i)))
      plotLayout()->remove(le);
  }
  plotLayout()->simplify();
}

void GenericPlot::addGradientLegend(QCPColorMap *colorMap)
{
  if (!colorMap)
    return;

  //add color scale
  QCPColorScale *colorScale = new QCPColorScale(this);
  plotLayout()->addElement(0, 1, colorScale);
  colorMap->setColorScale(colorScale);

  colorScale->setType(QCPAxis::atRight);
  colorScale->setRangeDrag(false);
  colorScale->setRangeZoom(false);
  colorScale->axis()->setTickLength(6,6);
  colorScale->axis()->setSubTickLength(2,1);
  colorScale->axis()->setLabel(colorMap->property("z_label").toString());
  colorScale->axis()->setNumberFormat("gbc");
  colorScale->axis()->setSubTicks(true);

  //readjust margins
  QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
  axisRect()->setMarginGroup(static_cast<QCP::MarginSide>(QCP::msBottom|QCP::msTop), marginGroup);
  colorScale->setMarginGroup(static_cast<QCP::MarginSide>(QCP::msBottom|QCP::msTop), marginGroup);
}

void GenericPlot::setShowMarkerLabels(bool sl)
{
  show_marker_labels_ = sl;
  //  replot_markers();
  rebuild_menu();
  replot();
}

void GenericPlot::setLineThickness(uint16_t th)
{
  line_thickness_ = th;
  for (int i=0; i < graphCount(); i++)
    set_graph_thickness(graph(i));
}

void GenericPlot::setGridStyle(QString grd)
{
  if ((grd == "No grid") || (grd == "Grid") || (grd == "Grid + subgrid"))
  {
    current_grid_style_ = grd;
    xAxis->grid()->setVisible(current_grid_style_ != "No grid");
    yAxis->grid()->setVisible(current_grid_style_ != "No grid");
    xAxis->grid()->setSubGridVisible(current_grid_style_ == "Grid + subgrid");
    yAxis->grid()->setSubGridVisible(current_grid_style_ == "Grid + subgrid");
    rebuild_menu();
  }
}

void GenericPlot::setScaleType(QString sct)
{
  if (!scale_types_.count(sct))
    return;

  this->setCursor(Qt::WaitCursor);
  current_scale_type_ = sct;

  bool has2d {false};
  for (int i=0; i < plottableCount(); ++i)
    if (QCPColorMap *cm = qobject_cast<QCPColorMap*>(plottable(i)))
    {
      has2d = true;
      cm->setDataScaleType(scale_types_[current_scale_type_]);
      //      cm->rescaleDataRange(true);
    }

  if (has2d)
  {
    for (int i=0; i < layerCount(); i++)
    {
      QCPLayer *this_layer = layer(i);
      for (auto &q : this_layer->children())
        if (QCPColorScale *colorScale = qobject_cast<QCPColorScale*>(q))
        {
          colorScale->axis()->setScaleType(scale_types_[current_scale_type_]);
          colorScale->rescaleDataRange(true);
        }
    }
    if (current_scale_type_ != sct)
      rescaleAxes();
  }
  else
    yAxis->setScaleType(scale_types_[current_scale_type_]);

  current_scale_type_ = sct;
  rebuild_menu();
  this->setCursor(Qt::ArrowCursor);
}

void GenericPlot::setPlotStyle(QString stl)
{
  this->setCursor(Qt::WaitCursor);
  current_plotStyle_ = stl;
  for (int i=0; i < graphCount(); i++)
    set_graph_style(graph(i), stl);
  rebuild_menu();
  //  replot();
  this->setCursor(Qt::ArrowCursor);
}

void GenericPlot::setGradient(QString grd)
{
  if (!gradients_.count(grd))
    return;

  this->setCursor(Qt::WaitCursor);
  current_gradient_ = grd;
  for (int i=0; i < plottableCount(); ++i)
    if (QCPColorMap *cm = qobject_cast<QCPColorMap*>(plottable(i)))
      cm->setGradient(gradients_[current_gradient_]);
  rebuild_menu();
  this->setCursor(Qt::ArrowCursor);
}





QSize GenericPlot::sizeHint() const
{
  if (always_square_) {
    QSize s = size();
    previous_height_ = s.height();

    int extra_width = 0, extra_height = 0;
    for (int i=0; i < layerCount(); i++) {
      QCPLayer *this_layer = layer(i);
      for (auto &q : this_layer->children()) {
        if (QCPColorScale *le = qobject_cast<QCPColorScale*>(q)) {
          QRect ler = le->outerRect();
          //extra_width += ler.width();
          /*} else if (QCPAxis *le = qobject_cast<QCPAxis*>(q)) {
          if (le->axisType() == QCPAxis::atBottom)
            extra_height += le->axisRect()->height();
          else if (le->axisType() == QCPAxis::atLeft)
            extra_width += le->axisRect()->width();*/
        } else if (QCPAxisRect *le = qobject_cast<QCPAxisRect*>(q)) {
          QMargins mar = le->margins();
          extra_width += (mar.left() + mar.right());
          extra_height += (mar.top() + mar.bottom());
        }
      }
    }

    s.setWidth(s.height() - extra_height + extra_width);
    s.setHeight(QCustomPlot::sizeHint().height());
    return s;
  } else
    return QCustomPlot::sizeHint();
}

void GenericPlot::resizeEvent(QResizeEvent * event)
{
  QCustomPlot::resizeEvent(event);

  // maybe this call should be scheduled for next iteration of event loop?
  if (always_square_ && (previous_height_ != height()))
    updateGeometry();
}

void GenericPlot::mousePressEvent(QMouseEvent *event)
{
  emit mousePress(event);

  DraggableTracer *trc = qobject_cast<DraggableTracer*>(itemAt(event->localPos(), true));
  if ((event->button() == Qt::LeftButton) && (trc != nullptr)) {
    trc->startMoving(event->localPos());
    return;
  }

  QCustomPlot::mousePressEvent(event);
}


void GenericPlot::mouseMoveEvent(QMouseEvent *event)
{
  emit mouseMove(event);
  double co_x, co_y;

  co_x = xAxis->pixelToCoord(event->x());
  co_y = yAxis->pixelToCoord(event->y());

  QVariant details;
  QCPLayerable *clickedLayerable = layerableAt(event->pos(), true, &details);
  if (QCPColorMap *ap = qobject_cast<QCPColorMap*>(clickedLayerable)) {
    int x = co_x, y = co_y;
    ap->data()->coordToCell(co_x, co_y, &x, &y);
    emit mouseHover(static_cast<double>(x), static_cast<double>(y));
  } //else
  //emit mouseHover(co_x, co_y);

  if (event->buttons() == Qt::NoButton) {
    DraggableTracer *trc = qobject_cast<DraggableTracer*>(itemAt(event->localPos(), false));
    Button *button = qobject_cast<Button*>(itemAt(event->localPos(), false));

    if (trc && trc->visible())
      setCursor(Qt::SizeHorCursor);
    else if (button && button->visible())
      setCursor(Qt::PointingHandCursor);
    else
      unsetCursor();
  }

  QCustomPlot::mouseMoveEvent(event);
}

void GenericPlot::mouseReleaseEvent(QMouseEvent *event)
{
  emit mouseRelease(event);

  //DBG << "GenericPlot::mouseReleaseEvent";

  if ((mMousePressPos-event->pos()).manhattanLength() < 5) {
    double co_x, co_y;

    co_x = xAxis->pixelToCoord(event->x());
    co_y = yAxis->pixelToCoord(event->y());

    //DBG << "Custom plot mouse released at coords: " << co_x << ", " << co_y;

    QCPAbstractItem *ai = qobject_cast<QCPAbstractItem*>(itemAt(event->localPos(), false));
    if (ai != nullptr) {
      emit clickedAbstractItem(ai);
      //      DBG << "clicked abstract item";
    }

    QVariant details;
    QCPLayerable *clickedLayerable = layerableAt(event->pos(), false, &details);
    if (QCPColorMap *ap = qobject_cast<QCPColorMap*>(clickedLayerable)) {
      int xx, yy;
      ap->data()->coordToCell(co_x, co_y, &xx, &yy);
      //DBG << "Corrected to cell : " << xx << ", " << yy;
      emit mouseClicked(static_cast<double>(xx), static_cast<double>(yy), event, true); //true?
    } else if (ai == nullptr) {
      emit mouseClicked(co_x, co_y, event, false);
      //DBG << "Uncorrected click coords";
    }
  }
  QCustomPlot::mouseReleaseEvent(event);
}

void GenericPlot::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Shift) {
    emit shiftStateChanged(true);
  }
  QCustomPlot::keyPressEvent(event);
}

void GenericPlot::keyReleaseEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Shift) {
    emit shiftStateChanged(false);
  }
  QCustomPlot::keyReleaseEvent(event);
}

void GenericPlot::setColorScheme(QColor fore, QColor back, QColor grid1, QColor grid2)
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

void GenericPlot::plot_buttons()
{
  Button *overlayButton;
  Button *newButton;

  if (visible_options_ & ShowOptions::zoom)
  {
    newButton = new Button(this,
                           QPixmap(":/icons/oxy/22/view_fullscreen.png"),
                           "reset_scales", "Zoom out",
                           Qt::AlignBottom | Qt::AlignRight);
    newButton->setClipToAxisRect(false);
    newButton->topLeft->setType(QCPItemPosition::ptAbsolute);
    newButton->topLeft->setCoords(5, 5);
    overlayButton = newButton;
  }

  if (!options_menu_.isEmpty())
  {
    newButton = new Button(this, QPixmap(":/icons/oxy/22/view_statistics.png"),
                           "options", "Style options",
                           Qt::AlignBottom | Qt::AlignRight);

    newButton->setClipToAxisRect(false);
    newButton->topLeft->setParentAnchor(overlayButton->bottomLeft);
    newButton->topLeft->setCoords(0, 5);
    overlayButton = newButton;
  }

  if (visible_options_ & ShowOptions::save)
  {
    newButton = new Button(this,
                           QPixmap(":/icons/oxy/22/document_save.png"),
                           "export", "Export plot",
                           Qt::AlignBottom | Qt::AlignRight);
    newButton->setClipToAxisRect(false);
    newButton->topLeft->setParentAnchor(overlayButton->bottomLeft);
    newButton->topLeft->setCoords(0, 5);
  }
}

void GenericPlot::exportPlot(QAction* choice)
{
  QString filter = choice->text() + "(*." + choice->text() + ")";
  QString fileName = CustomSaveFileDialog(this, "Export plot",
                                          QStandardPaths::locate(QStandardPaths::HomeLocation, ""),
                                          filter);
  if (!validateFile(this, fileName, true))
    return;

  rescaleEverything(5, 2, 20, false);

  QFileInfo file(fileName);
  if (file.suffix() == "png")
    savePng(fileName,0,0,1,100);
  else if (file.suffix() == "jpg")
    saveJpg(fileName,0,0,1,100);
  else if (file.suffix() == "bmp")
    saveBmp(fileName);
  else if (file.suffix() == "pdf")
    savePdf(fileName, true);

  rescaleEverything(-5, -2, -20, true);
  replot();
  updateGeometry();
}

void GenericPlot::rescaleEverything(int fontUpscale, int plotThicken, int marginUpscale,
                                    bool buttons_visible)
{
  for (int i = 0; i < graphCount(); ++i)
  {
    QCPGraph *graph = this->graph(i);
    QPen pen = graph->pen();
    pen.setWidth(pen.width() + plotThicken);
    graph->setPen(pen);
  }
  for (int i = 0; i < itemCount(); ++i)
  {
    QCPAbstractItem* itm = item(i);
    if (QCPItemLine *line = qobject_cast<QCPItemLine*>(itm))
    {
      QCPLineEnding head = line->head();
      head.setWidth(head.width() + fontUpscale);
      head.setLength(head.length() + fontUpscale);
      line->setHead(head);
      line->start->setCoords(0, -50);
      line->end->setCoords(0, -15);
    }
    else if (QCPItemText *txt = qobject_cast<QCPItemText*>(itm))
    {
      txt->setFont(rescaleFont(txt->font(), fontUpscale));
      txt->position->setCoords(0, -50);
    }
    else if (Button *btn = qobject_cast<Button*>(itm))
      btn->setVisible(buttons_visible);
  }

  bool has_colormap {false};

  for (int i=0; i < layerCount(); i++)
  {
    QCPLayer *this_layer = layer(i);
    for (auto &q : this_layer->children()) {
      if (QCPColorScale *cs = qobject_cast<QCPColorScale*>(q))
      {
        cs->axis()->setLabelFont(rescaleFont(cs->axis()->labelFont(), fontUpscale));
        cs->axis()->setTickLabelFont(rescaleFont(cs->axis()->tickLabelFont(), fontUpscale));
        cs->axis()->setPadding(cs->axis()->padding() + marginUpscale);
        has_colormap = true;
      }
      else if (QCPAxisRect *ar = qobject_cast<QCPAxisRect*>(q))
      {
        for (auto ax : ar->axes())
        {
          ax->setLabelFont(rescaleFont(ax->labelFont(), fontUpscale));
          ax->setTickLabelFont(rescaleFont(ax->tickLabelFont(), fontUpscale));
          ax->setPadding(ax->padding() + marginUpscale);
        }
      }
    }
  }

  if (has_colormap)
    setScaleType(current_scale_type_);
}

QFont GenericPlot::rescaleFont(QFont font, double size_offset)
{
  font.setPointSize(font.pointSize() + size_offset);
  return font;
}

void GenericPlot::set_graph_thickness(QCPGraph* graph)
{
  if (visible_options_ & ShowOptions::thickness)
  {
    QPen pen = graph->pen();
    pen.setWidth(line_thickness_);
    graph->setPen(pen);
  }
}

void GenericPlot::set_graph_style(QCPGraph* graph, QString style)
{
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

  set_graph_thickness(graph);
}



void GenericPlot::optionsChanged(QAction* action)
{
  this->setCursor(Qt::WaitCursor);
  QString choice = action->text();

  if (scale_types_.count(choice))
    setScaleType(choice);
  else if (grid_styles_.count(choice))
    setGridStyle(choice);
  else if (choice == "Show marker labels")
    setShowMarkerLabels(!show_marker_labels_);
  else if (choice == "Antialiased")
    setAntialiased(!antialiased_);
  else if (plot_styles_.count(choice))
    setPlotStyle(choice);
  else if (gradients_.count(choice))
    setGradient(choice);
  else if (choice == "Show gradient scale")
    setShowGradientLegend(!show_gradient_legend_);
  else if ((choice == "1") || (choice == "2") || (choice == "3"))
    setLineThickness(choice.toInt());

  rebuild_menu();
  this->setCursor(Qt::ArrowCursor);
}

void GenericPlot::rebuild_menu()
{
  options_menu_.clear();

  if (visible_options_ & ShowOptions::scale)
  {
    for (auto &s : scale_types_)
      options_menu_.addAction(s.first);
    options_menu_.addSeparator();
  }

  if (visible_options_ & ShowOptions::labels)
  {
    options_menu_.addAction("Show marker labels");
    options_menu_.addSeparator();
  }

  if (visible_options_ & ShowOptions::style)
  {
    for (auto &s : plot_styles_)
      options_menu_.addAction(s);
    options_menu_.addSeparator();
  }

  if (visible_options_ & ShowOptions::thickness)
  {
    options_menu_.addAction("1");
    options_menu_.addAction("2");
    options_menu_.addAction("3");
    options_menu_.addSeparator();
  }

  if (visible_options_ & ShowOptions::gradients)
  {
    if (!gradients_.empty())
    {
      options_menu_.addAction("Show gradient scale");
      options_menu_.addSeparator();
    }
    for (auto &q : gradients_)
      options_menu_.addAction(q.first);
    options_menu_.addSeparator();
  }

  if (visible_options_ & ShowOptions::grid)
  {
    for (auto &s : grid_styles_)
      options_menu_.addAction(s);
    options_menu_.addSeparator();
  }

  if (visible_options_ & ShowOptions::dither)
  {
    options_menu_.addAction("Antialiased");
  }

  for (auto &q : options_menu_.actions())
  {
    q->setCheckable(true);
    q->setChecked((q->text() == current_scale_type_) ||
                  (q->text() == current_plotStyle_) ||
                  (q->text() == current_grid_style_) ||
                  (q->text() == current_gradient_) ||
                  (q->text() == QString::number(line_thickness_)) ||
                  ((q->text() == "Show marker labels") && show_marker_labels_) ||
                  ((q->text() == "Show gradient scale") && show_gradient_legend_)||
                  ((q->text() == "Antialiased") && antialiased_));
  }
}


}