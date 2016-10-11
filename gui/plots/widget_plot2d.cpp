#include "widget_plot2d.h"
#include "qt_util.h"
#include "CustomLogger.h"
#include "qcp_overlay_button.h"

WidgetPlot2D::WidgetPlot2D(QWidget *parent) :
  QSquareCustomPlot(parent),
  scale_types_({{"Linear", QCPAxis::stLinear}, {"Logarithmic", QCPAxis::stLogarithmic}}),
  gradients_({{"Grayscale", QCPColorGradient::gpGrayscale}, {"Hot",  QCPColorGradient::gpHot},
{"Cold", QCPColorGradient::gpCold}, {"Night", QCPColorGradient::gpNight},
{"Candy", QCPColorGradient::gpCandy}, {"Geography", QCPColorGradient::gpGeography},
{"Ion", QCPColorGradient::gpIon}, {"Thermal", QCPColorGradient::gpThermal},
{"Polar", QCPColorGradient::gpPolar}, {"Spectrum", QCPColorGradient::gpSpectrum},
{"Jet", QCPColorGradient::gpJet}, {"Hues", QCPColorGradient::gpHues}})
{

  current_gradient_ = "Night";
  current_scale_type_ = "Linear";
  show_gradient_scale_ = false;
  antialiased_ = false;
  show_labels_ = true;

//  setColorScheme(Qt::white, Qt::black, QColor(144, 144, 144), QColor(80, 80, 80));
  setColorScheme(Qt::black, Qt::white, QColor(112, 112, 112), QColor(170, 170, 170));

  xAxis->grid()->setVisible(true);
  yAxis->grid()->setVisible(true);
  xAxis->grid()->setSubGridVisible(true);
  yAxis->grid()->setSubGridVisible(true);

  //color theme setup
  marker_looks.themes["Grayscale"] = QPen(Qt::cyan, 1);
  marker_looks.themes["Hot"] = QPen(Qt::cyan, 1);
  marker_looks.themes["Cold"] = QPen(Qt::yellow, 1);
  marker_looks.themes["Night"] = QPen(Qt::red, 1);
  marker_looks.themes["Candy"] = QPen(Qt::red, 1);
  marker_looks.themes["Geography"] = QPen(Qt::yellow, 1);
  marker_looks.themes["Ion"] = QPen(Qt::magenta, 1);
  marker_looks.themes["Thermal"] = QPen(Qt::cyan, 1);
  marker_looks.themes["Polar"] = QPen(Qt::green, 1);
  marker_looks.themes["Spectrum"] = QPen(Qt::cyan, 1);
  marker_looks.themes["Jet"] = QPen(Qt::darkMagenta, 1);
  marker_looks.themes["Hues"] = QPen(Qt::black, 1);

  //colormap setup
  setAlwaysSquare(true);
  colorMap = new QCPColorMap(xAxis, yAxis);
  colorMap->clearData();
  addPlottable(colorMap);
  colorMap->setInterpolate(antialiased_);
  colorMap->setTightBoundary(false);
  axisRect()->setupFullAxesBox();
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  setNoAntialiasingOnDrag(true);

  colorMap->setGradient(gradients_[current_gradient_]);
  colorMap->setDataScaleType(scale_types_[current_scale_type_]);
  colorMap->rescaleDataRange(true);
  connect(this, SIGNAL(mouse_clicked(double,double,QMouseEvent*, bool)), this, SLOT(plot_2d_mouse_clicked(double,double,QMouseEvent*, bool)));
  //connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(clicked_plottable(QCPAbstractPlottable*)));
  connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selection_changed()));
  connect(this, SIGNAL(clickedAbstractItem(QCPAbstractItem*)), this, SLOT(clicked_item(QCPAbstractItem*)));

  menuExportFormat.addAction("png");
  menuExportFormat.addAction("jpg");
  menuExportFormat.addAction("pdf");
  menuExportFormat.addAction("bmp");
  connect(&menuExportFormat, SIGNAL(triggered(QAction*)), this, SLOT(exportRequested(QAction*)));
  connect(&menuOptions, SIGNAL(triggered(QAction*)), this, SLOT(optionsChanged(QAction*)));

  build_menu();
}

void WidgetPlot2D::set_zoom_drag(Qt::Orientation o)
{
  yAxis->axisRect()->setRangeDrag(o);
  yAxis->axisRect()->setRangeZoom(o);
}


void WidgetPlot2D::clicked_item(QCPAbstractItem* itm) {
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

void WidgetPlot2D::build_menu() {
  menuOptions.clear();
  for (auto &q : gradients_)
    menuOptions.addAction(q.first);

  menuOptions.addSeparator();
  menuOptions.addAction("Logarithmic");
  menuOptions.addAction("Linear");

  menuOptions.addSeparator();
  menuOptions.addAction("Show gradient scale");

  menuOptions.addSeparator();
  menuOptions.addAction("Show marker labels");

  menuOptions.addSeparator();
  menuOptions.addAction("Antialiased");

  for (auto &q : menuOptions.actions()) {
    q->setCheckable(true);
    q->setChecked((q->text() == current_scale_type_) ||
                  (q->text() == current_gradient_) ||
                  ((q->text() == "Show gradient scale") && show_gradient_scale_)||
                  ((q->text() == "Show marker labels") && show_labels_)||
                  ((q->text() == "Antialiased") && antialiased_));
  }
}

void WidgetPlot2D::set_antialiased(bool anti) {
  antialiased_ = anti;
  colorMap->setInterpolate(antialiased_);
  build_menu();
}

void WidgetPlot2D::optionsChanged(QAction* action) {
  this->setCursor(Qt::WaitCursor);
  QString choice = action->text();
  if (scale_types_.count(choice)) {
    current_scale_type_ = choice;
    colorMap->setDataScaleType(scale_types_[current_scale_type_]);
    colorMap->rescaleDataRange(true);
    replot();
  } else if (gradients_.count(choice)) {
    current_gradient_ = choice;
    colorMap->setGradient(gradients_[current_gradient_]);
    replot_markers();
  } else if (choice == "Show gradient scale") {
    show_gradient_scale_ = !show_gradient_scale_;
    toggle_gradient_scale();
  } else if (choice == "Show marker labels") {
    show_labels_ = !show_labels_;
    replot_markers();
  } else if (choice == "Antialiased") {
    antialiased_= !antialiased_;
    colorMap->setInterpolate(antialiased_);
  }

  build_menu();
  this->setCursor(Qt::ArrowCursor);
}


void WidgetPlot2D::selection_changed() {
  emit stuff_selected();
}


std::list<MarkerBox2D> WidgetPlot2D::get_selected_boxes() {
  std::list<MarkerBox2D> selection;
  for (auto &q : selectedItems())
    if (QCPItemRect *b = qobject_cast<QCPItemRect*>(q)) {
      MarkerBox2D box;
      box.x1 = b->property("chan_x").toDouble();
      box.y1 = b->property("chan_y").toDouble();
      selection.push_back(box);
      //DBG << "found selected " << txt->property("true_value").toDouble() << " chan=" << txt->property("chan_value").toDouble();
    }
  return selection;
}


void WidgetPlot2D::set_boxes(std::list<MarkerBox2D> boxes) {
  boxes_ = boxes;
}


void WidgetPlot2D::reset_content() {
  colorMap->clearData();
  boxes_.clear();
//  labels_.clear();
}

void WidgetPlot2D::refresh()
{
  replot();
}

void WidgetPlot2D::replot_markers() {

  clearItems();

  QPen pen = marker_looks.get_pen(current_gradient_);

  QColor cc = pen.color();
  cc.setAlpha(169);
  pen.setColor(cc);
  pen.setWidth(3);

  QCPItemRect *box;

  int selectables = 0;
  for (auto &q : boxes_) {

    //    if (!q.visible)
//      continue;
    box = new QCPItemRect(this);
    box->setSelectable(q.selectable);
    box->setPen(q.border);
//    box->setSelectedPen(pen);
    box->setBrush(QBrush(q.fill));
    box->setSelected(q.selected);
    QColor sel = box->selectedPen().color();
    box->setSelectedBrush(QBrush(QColor::fromHsv(sel.hsvHue(), sel.saturation(), sel.value(), 48)));

    box->setProperty("chan_x", q.x1);
    box->setProperty("chan_y", q.y1);
    box->topLeft->setCoords(q.x1, q.y1);
    box->bottomRight->setCoords(q.x2, q.y2);
    addItem(box);

    if (q.selectable)
      selectables++;

    if (!q.label.isEmpty())
    {
      QCPItemText *labelItem = new QCPItemText(this);
      labelItem->setText(q.label);
      labelItem->setProperty("chan_x", q.x1);
      labelItem->setProperty("chan_y", q.y1);
      labelItem->position->setType(QCPItemPosition::ptPlotCoords);
      labelItem->position->setCoords(q.x1, q.y2);

      labelItem->setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
      labelItem->setFont(QFont("Helvetica", 14));
      labelItem->setSelectable(q.selectable);
      labelItem->setSelected(q.selected);

      labelItem->setColor(q.border);
      labelItem->setPen(QPen(q.border));
      labelItem->setBrush(QBrush(Qt::white));

      QColor sel = labelItem->selectedColor();
      QPen selpen(QColor::fromHsv(sel.hsvHue(), sel.saturation(), sel.value(), 255));
      selpen.setWidth(3);
      labelItem->setSelectedPen(selpen);
      labelItem->setSelectedBrush(QBrush(Qt::white));

      labelItem->setPadding(QMargins(1, 1, 1, 1));

      addItem(labelItem);
    }

  }

  if (selectables) {
    setInteraction(QCP::iSelectItems, true);
    setInteraction(QCP::iMultiSelect, false);
  } else {
    setInteraction(QCP::iSelectItems, false);
    setInteraction(QCP::iMultiSelect, false);
  }

  plotButtons();

  replot();
}

void WidgetPlot2D::plotButtons() {
  QCPOverlayButton *overlayButton;
  QCPOverlayButton *newButton;

  newButton = new QCPOverlayButton(this,
                                   QPixmap(":/icons/oxy/22/view_fullscreen.png"),
                                   "reset_scales", "Zoom out",
                                   Qt::AlignBottom | Qt::AlignRight);
  newButton->setClipToAxisRect(false);
  newButton->topLeft->setType(QCPItemPosition::ptAbsolute);
  newButton->topLeft->setCoords(5, 5);
  addItem(newButton);
  overlayButton = newButton;

  if (!menuOptions.isEmpty()) {
    newButton = new QCPOverlayButton(this, QPixmap(":/icons/oxy/22/view_statistics.png"),
                                     "options", "Style options",
                                     Qt::AlignBottom | Qt::AlignRight);

    newButton->setClipToAxisRect(false);
    newButton->topLeft->setParentAnchor(overlayButton->bottomLeft);
    newButton->topLeft->setCoords(0, 5);
    addItem(newButton);
    overlayButton = newButton;
  }

//  if (visible_options_ & ShowOptions::save) {
    newButton = new QCPOverlayButton(this,
                                     QPixmap(":/icons/oxy/22/document_save.png"),
                                     "export", "Export plot",
                                     Qt::AlignBottom | Qt::AlignRight);
    newButton->setClipToAxisRect(false);
    newButton->topLeft->setParentAnchor(overlayButton->bottomLeft);
    newButton->topLeft->setCoords(0, 5);
    addItem(newButton);
//    overlayButton = newButton;
//  }
}

void WidgetPlot2D::update_plot(uint64_t sizex, uint64_t sizey, const EntryList &spectrum_data)
{
//  DBG << "2d size " << sizex << "x" << sizey << " list " << spectrum_data.size();

  clearGraphs();
  colorMap->clearData();
  setAlwaysSquare(sizex == sizey);
  if (sizex == sizey)
  {
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
//    verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
  }
  else
  {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//    verticalLayout->setSizeConstraint(QLayout::SetNoConstraint);
  }

  if ((sizex > 0) && (sizey > 0) && (spectrum_data.size())) {
    colorMap->data()->setSize(sizex, sizey);
    for (auto it : spectrum_data)
      if ((it.first.size() > 1) && (it.first[0] >= 0) && (it.first[1] >= 0))
        colorMap->data()->setCell(it.first[0], it.first[1], it.second);
    colorMap->rescaleDataRange(true);
    updateGeometry();
  } else {
    clearGraphs();
    colorMap->clearData();
    colorMap->keyAxis()->setLabel("");
    colorMap->valueAxis()->setLabel("");
  }

  replot_markers();
}

void WidgetPlot2D::set_axes(QString xlabel, double x1, double x2,
                            QString ylabel, double y1, double y2,
                            QString zlabel)
{
  Z_label_ = zlabel;
  for (int i=0; i < plotLayout()->elementCount(); i++)
    if (QCPColorScale *le = qobject_cast<QCPColorScale*>(plotLayout()->elementAt(i)))
      le->axis()->setLabel(zlabel);

  colorMap->keyAxis()->setLabel(xlabel);
  colorMap->valueAxis()->setLabel(ylabel);
  colorMap->data()->setRange(QCPRange(x1, x2),
                             QCPRange(y1, y2));
  rescaleAxes();
}

void WidgetPlot2D::plot_2d_mouse_clicked(double x, double y, QMouseEvent *event, bool /*channels*/)
{
  emit markers_set(x, y, event->button() == Qt::LeftButton);
}


void WidgetPlot2D::zoom_out()
{
  this->setCursor(Qt::WaitCursor);
  rescaleAxes();
  replot();

  double margin = 0.5;
  double x_lower = xAxis->range().lower;
  double x_upper = xAxis->range().upper;
  double y_lower = yAxis->range().lower;
  double y_upper = yAxis->range().upper;
  xAxis->setRange(x_lower - margin, x_upper + margin);
  yAxis->setRange(y_lower - margin, y_upper + margin);

  replot();


  this->setCursor(Qt::ArrowCursor);
}


void WidgetPlot2D::toggle_gradient_scale()
{
  if (show_gradient_scale_) {
    //add color scale
    QCPColorScale *colorScale = new QCPColorScale(this);
    plotLayout()->addElement(0, 1, colorScale);
    colorScale->setType(QCPAxis::atRight);
    colorMap->setColorScale(colorScale);

//    colorScale->axis()->setBasePen(QPen(Qt::white, 1));
//    colorScale->axis()->setTickPen(QPen(Qt::white, 1));
//    colorScale->axis()->setSubTickPen(QPen(Qt::white, 1));
//    colorScale->axis()->setTickLabelColor(Qt::white);
//    colorScale->axis()->setLabelColor(Qt::white);

    colorScale->axis()->setLabel(Z_label_);

    //readjust margins
    QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
    axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    colorMap->setGradient(gradients_[current_gradient_]);
    colorMap->setDataScaleType(scale_types_[current_scale_type_]);
    colorMap->rescaleDataRange(true);

    colorScale->axis()->setScaleLogBase(10);
    colorScale->axis()->setNumberFormat("gbc");
    colorScale->axis()->setNumberPrecision(0);
    colorScale->axis()->setRangeLower(1);

    replot();
    updateGeometry();
  } else {
    for (int i=0; i < plotLayout()->elementCount(); i++) {
      if (QCPColorScale *le = qobject_cast<QCPColorScale*>(plotLayout()->elementAt(i)))
        plotLayout()->remove(le);
    }
    plotLayout()->simplify();
    updateGeometry();
  }
}

void WidgetPlot2D::set_scale_type(QString sct) {
  this->setCursor(Qt::WaitCursor);
  current_scale_type_ = sct;
  colorMap->setDataScaleType(scale_types_[current_scale_type_]);
  colorMap->rescaleDataRange(true);
  replot();
  build_menu();
  this->setCursor(Qt::ArrowCursor);
}

QString WidgetPlot2D::scale_type() {
  return current_scale_type_;
}

void WidgetPlot2D::set_gradient(QString grd) {
  this->setCursor(Qt::WaitCursor);
  current_gradient_ = grd;
  colorMap->setGradient(gradients_[current_gradient_]);
  replot_markers();
  build_menu();
  this->setCursor(Qt::ArrowCursor);
}

QString WidgetPlot2D::gradient() {
  return current_gradient_;
}

void WidgetPlot2D::set_show_legend(bool show) {
  show_gradient_scale_ = show;
  toggle_gradient_scale();
  build_menu();
}

bool WidgetPlot2D::show_legend() {
  return show_gradient_scale_;
}

void WidgetPlot2D::exportRequested(QAction* choice) {
  QString filter = choice->text() + "(*." + choice->text() + ")";
  QString fileName = CustomSaveFileDialog(this, "Export plot",
                                          QStandardPaths::locate(QStandardPaths::HomeLocation, ""),
                                          filter);
  if (validateFile(this, fileName, true))
  {

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

//    plot_rezoom();
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
//    plot_rezoom();
    for (int i = 0; i < itemCount(); ++i) {
      QCPAbstractItem* itm = item(i);
      if (QCPOverlayButton *btn = qobject_cast<QCPOverlayButton*>(itm))
        btn->setVisible(true);
    }

    replot();
  }
}

void WidgetPlot2D::setColorScheme(QColor fore, QColor back, QColor grid1, QColor grid2)
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
