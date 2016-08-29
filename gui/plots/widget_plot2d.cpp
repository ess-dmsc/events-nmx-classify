#include "widget_plot2d.h"
#include "ui_widget_plot2d.h"
#include "qt_util.h"
#include "CustomLogger.h"

WidgetPlot2D::WidgetPlot2D(QWidget *parent) :
  QWidget(parent),
  scale_types_({{"Linear", QCPAxis::stLinear}, {"Logarithmic", QCPAxis::stLogarithmic}}),
  gradients_({{"Grayscale", QCPColorGradient::gpGrayscale}, {"Hot",  QCPColorGradient::gpHot},
{"Cold", QCPColorGradient::gpCold}, {"Night", QCPColorGradient::gpNight},
{"Candy", QCPColorGradient::gpCandy}, {"Geography", QCPColorGradient::gpGeography},
{"Ion", QCPColorGradient::gpIon}, {"Thermal", QCPColorGradient::gpThermal},
{"Polar", QCPColorGradient::gpPolar}, {"Spectrum", QCPColorGradient::gpSpectrum},
{"Jet", QCPColorGradient::gpJet}, {"Hues", QCPColorGradient::gpHues}}),
  ui(new Ui::WidgetPlot2D)
{
  ui->setupUi(this);

  current_gradient_ = "Night";
  current_scale_type_ = "Linear";
  show_gradient_scale_ = false;
  antialiased_ = false;
  show_labels_ = true;

  setColorScheme(Qt::white, Qt::black, QColor(144, 144, 144), QColor(80, 80, 80));
  ui->plot->xAxis->grid()->setVisible(true);
  ui->plot->yAxis->grid()->setVisible(true);
  ui->plot->xAxis->grid()->setSubGridVisible(true);
  ui->plot->yAxis->grid()->setSubGridVisible(true);

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
  ui->plot->setAlwaysSquare(true);
  colorMap = new QCPColorMap(ui->plot->xAxis, ui->plot->yAxis);
  colorMap->clearData();
  ui->plot->addPlottable(colorMap);
  colorMap->setInterpolate(antialiased_);
  colorMap->setTightBoundary(false);
  ui->plot->axisRect()->setupFullAxesBox();
  ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  ui->plot->setNoAntialiasingOnDrag(true);

  colorMap->setGradient(gradients_[current_gradient_]);
  colorMap->setDataScaleType(scale_types_[current_scale_type_]);
  colorMap->rescaleDataRange(true);
  connect(ui->plot, SIGNAL(mouse_upon(double,double)), this, SLOT(plot_2d_mouse_upon(double,double)));
  connect(ui->plot, SIGNAL(mouse_clicked(double,double,QMouseEvent*, bool)), this, SLOT(plot_2d_mouse_clicked(double,double,QMouseEvent*, bool)));
  //connect(ui->plot, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(clicked_plottable(QCPAbstractPlottable*)));
  connect(ui->plot, SIGNAL(selectionChangedByUser()), this, SLOT(selection_changed()));
  connect(ui->plot, SIGNAL(clickedAbstractItem(QCPAbstractItem*)), this, SLOT(clicked_item(QCPAbstractItem*)));

  menuExportFormat.addAction("png");
  menuExportFormat.addAction("jpg");
  menuExportFormat.addAction("pdf");
  menuExportFormat.addAction("bmp");
  connect(&menuExportFormat, SIGNAL(triggered(QAction*)), this, SLOT(exportRequested(QAction*)));
  connect(&menuOptions, SIGNAL(triggered(QAction*)), this, SLOT(optionsChanged(QAction*)));

  build_menu();
  plot_2d_mouse_upon(0, 0);
}

void WidgetPlot2D::set_zoom_drag(Qt::Orientation o)
{
  ui->plot->yAxis->axisRect()->setRangeDrag(o);
  ui->plot->yAxis->axisRect()->setRangeZoom(o);
}


void WidgetPlot2D::clicked_item(QCPAbstractItem* itm) {
  if (QCPItemPixmap *pix = qobject_cast<QCPItemPixmap*>(itm)) {
//    QPoint p = this->mapFromGlobal(QCursor::pos());
    QString name = pix->property("button_name").toString();
    if (name == "options") {
      menuOptions.exec(QCursor::pos());
    } else if (name == "export") {
      menuExportFormat.exec(QCursor::pos());
    } else if (name == "reset_scales") {
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
    ui->plot->replot();
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


WidgetPlot2D::~WidgetPlot2D()
{
  delete ui;
}

void WidgetPlot2D::selection_changed() {
  emit stuff_selected();
}


std::list<MarkerBox2D> WidgetPlot2D::get_selected_boxes() {
  std::list<MarkerBox2D> selection;
  for (auto &q : ui->plot->selectedItems())
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
  ui->plot->replot();
}

void WidgetPlot2D::replot_markers() {

  ui->plot->clearItems();

  QPen pen = marker_looks.get_pen(current_gradient_);

  QColor cc = pen.color();
  cc.setAlpha(169);
  pen.setColor(cc);
  pen.setWidth(3);

  QPen pen_strong = pen;
  cc.setAlpha(255);
  cc.setHsv((cc.hsvHue() + 180) % 180, 255, 128, 255);
  pen_strong.setColor(cc);
  pen_strong.setWidth(3);

  QPen pen_strong2 = pen_strong;
  cc.setAlpha(64);
  pen_strong2.setColor(cc);

  QCPItemRect *box;

  int selectables = 0;
  for (auto &q : boxes_) {
    QColor fill_color = pen.color();
    if (!q.mark_center)
      fill_color.setAlpha(48);

    //    if (!q.visible)
//      continue;
    box = new QCPItemRect(ui->plot);
    box->setSelectable(q.selectable);
    if (q.selectable) {
      box->setPen(pen);
//      box->setSelectedPen(pen);
      box->setBrush(QBrush(fill_color));
      box->setSelected(q.selected);
      QColor sel = box->selectedPen().color();
      box->setSelectedBrush(QBrush(QColor::fromHsv(sel.hsvHue(), sel.saturation(), sel.value(), 48)));
    } else {
      box->setPen(pen);
    }

    box->setProperty("chan_x", q.x1);
    box->setProperty("chan_y", q.y1);
    box->topLeft->setCoords(q.x1, q.y1);
    box->bottomRight->setCoords(q.x2, q.y2);
    ui->plot->addItem(box);

    if (q.selectable)
      selectables++;

    if (!q.label.isEmpty())
    {
      QCPItemText *labelItem = new QCPItemText(ui->plot);
      labelItem->setText(q.label);
      labelItem->setProperty("chan_x", q.x1);
      labelItem->setProperty("chan_y", q.y1);
      labelItem->position->setType(QCPItemPosition::ptPlotCoords);
      labelItem->position->setCoords(q.x1, q.y2);

      labelItem->setPositionAlignment(Qt::AlignBottom|Qt::AlignLeft);
      labelItem->setFont(QFont("Helvetica", 14));
      labelItem->setSelectable(q.selectable);
      labelItem->setSelected(q.selected);

      labelItem->setColor(pen_strong.color());
      labelItem->setPen(pen_strong);
      labelItem->setBrush(QBrush(Qt::white));

      QColor sel = labelItem->selectedColor();
      QPen selpen(QColor::fromHsv(sel.hsvHue(), sel.saturation(), sel.value(), 255));
      selpen.setWidth(3);
      labelItem->setSelectedPen(selpen);
      labelItem->setSelectedBrush(QBrush(Qt::white));

      labelItem->setPadding(QMargins(1, 1, 1, 1));

      ui->plot->addItem(labelItem);
    }

  }

  if (selectables) {
    ui->plot->setInteraction(QCP::iSelectItems, true);
    ui->plot->setInteraction(QCP::iMultiSelect, false);
  } else {
    ui->plot->setInteraction(QCP::iSelectItems, false);
    ui->plot->setInteraction(QCP::iMultiSelect, false);
  }


  QCPItemPixmap *overlayButton;

  overlayButton = new QCPItemPixmap(ui->plot);
  overlayButton->setClipToAxisRect(false);
  overlayButton->setPixmap(QPixmap(":/icons/oxy/16/view_fullscreen.png"));
  overlayButton->topLeft->setType(QCPItemPosition::ptAbsolute);
  overlayButton->topLeft->setCoords(5, 5);
  overlayButton->bottomRight->setParentAnchor(overlayButton->topLeft);
  overlayButton->bottomRight->setCoords(16, 16);
  overlayButton->setScaled(true);
  overlayButton->setSelectable(false);
  overlayButton->setProperty("button_name", QString("reset_scales"));
  overlayButton->setProperty("tooltip", QString("Reset plot scales"));
  ui->plot->addItem(overlayButton);

  if (!menuOptions.isEmpty()) {
    QCPItemPixmap *newButton = new QCPItemPixmap(ui->plot);
    newButton->setClipToAxisRect(false);
    newButton->setPixmap(QPixmap(":/icons/oxy/16/view_statistics.png"));
    newButton->topLeft->setType(QCPItemPosition::ptAbsolute);
    newButton->topLeft->setParentAnchor(overlayButton->bottomLeft);
    newButton->topLeft->setCoords(0, 5);
    newButton->bottomRight->setParentAnchor(newButton->topLeft);
    newButton->bottomRight->setCoords(16, 16);
    newButton->setScaled(false);
    newButton->setSelectable(false);
    newButton->setProperty("button_name", QString("options"));
    newButton->setProperty("tooltip", QString("Style options"));
    ui->plot->addItem(newButton);
    overlayButton = newButton;
  }

  QCPItemPixmap *newButton = new QCPItemPixmap(ui->plot);
  newButton->setClipToAxisRect(false);
  newButton->setPixmap(QPixmap(":/icons/oxy/16/document_save.png"));
  newButton->topLeft->setType(QCPItemPosition::ptAbsolute);
  newButton->topLeft->setParentAnchor(overlayButton->bottomLeft);
  newButton->topLeft->setCoords(0, 5);
  newButton->bottomRight->setParentAnchor(newButton->topLeft);
  newButton->bottomRight->setCoords(16, 16);
  newButton->setScaled(true);
  newButton->setSelectable(false);
  newButton->setProperty("button_name", QString("export"));
  newButton->setProperty("tooltip", QString("Export plot"));
  ui->plot->addItem(newButton);
  overlayButton = newButton;

  ui->plot->replot();
}

void WidgetPlot2D::update_plot(uint64_t sizex, uint64_t sizey, const EntryList &spectrum_data) {
  ui->plot->clearGraphs();
  colorMap->clearData();
  ui->plot->setAlwaysSquare(sizex == sizey);
  if (sizex == sizey)
  {
    ui->plot->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    ui->verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
  }
  else
  {
    ui->plot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ui->verticalLayout->setSizeConstraint(QLayout::SetNoConstraint);
  }

  if ((sizex > 0) && (sizey > 0) && (spectrum_data.size())) {
    colorMap->data()->setSize(sizex, sizey);
    for (auto it : spectrum_data)
      if ((it.first.size() > 1) && (it.first[0] >= 0) && (it.first[1] >= 0))
        colorMap->data()->setCell(it.first[0], it.first[1], it.second);
    colorMap->rescaleDataRange(true);
    ui->plot->updateGeometry();
  } else {
    ui->plot->clearGraphs();
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
  for (int i=0; i < ui->plot->plotLayout()->elementCount(); i++)
    if (QCPColorScale *le = qobject_cast<QCPColorScale*>(ui->plot->plotLayout()->elementAt(i)))
      le->axis()->setLabel(zlabel);

  colorMap->keyAxis()->setLabel(xlabel);
  colorMap->valueAxis()->setLabel(ylabel);
  colorMap->data()->setRange(QCPRange(x1, x2),
                             QCPRange(y1, y2));
  ui->plot->rescaleAxes();
}


void WidgetPlot2D::plot_2d_mouse_upon(double x, double y) {
//  colorMap->keyAxis()->setLabel("Energy (" + QString::fromStdString(calib_x_.units_) + "): " + QString::number(calib_x_.transform(x, bits_)));
//  colorMap->valueAxis()->setLabel("Energy (" + QString::fromStdString(calib_y_.units_) + "): " + QString::number(calib_y_.transform(y, bits_)));
//  ui->plot->replot();
}

void WidgetPlot2D::plot_2d_mouse_clicked(double x, double y, QMouseEvent *event, bool channels)
{
  emit markers_set(x, y, event->button() == Qt::LeftButton);
}


void WidgetPlot2D::zoom_out()
{
  this->setCursor(Qt::WaitCursor);
  ui->plot->rescaleAxes();
  ui->plot->replot();

  double margin = 0.5;
  double x_lower = ui->plot->xAxis->range().lower;
  double x_upper = ui->plot->xAxis->range().upper;
  double y_lower = ui->plot->yAxis->range().lower;
  double y_upper = ui->plot->yAxis->range().upper;
  ui->plot->xAxis->setRange(x_lower - margin, x_upper + margin);
  ui->plot->yAxis->setRange(y_lower - margin, y_upper + margin);

  ui->plot->replot();


  this->setCursor(Qt::ArrowCursor);
}


void WidgetPlot2D::toggle_gradient_scale()
{
  if (show_gradient_scale_) {
    //add color scale
    QCPColorScale *colorScale = new QCPColorScale(ui->plot);
    ui->plot->plotLayout()->addElement(0, 1, colorScale);
    colorScale->setType(QCPAxis::atRight);
    colorMap->setColorScale(colorScale);

    colorScale->axis()->setBasePen(QPen(Qt::white, 1));
    colorScale->axis()->setTickPen(QPen(Qt::white, 1));
    colorScale->axis()->setSubTickPen(QPen(Qt::white, 1));
    colorScale->axis()->setTickLabelColor(Qt::white);
    colorScale->axis()->setLabelColor(Qt::white);

    colorScale->axis()->setLabel(Z_label_);

    //readjust margins
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->plot);
    ui->plot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    colorMap->setGradient(gradients_[current_gradient_]);
    colorMap->setDataScaleType(scale_types_[current_scale_type_]);
    colorMap->rescaleDataRange(true);

    colorScale->axis()->setScaleLogBase(10);
    colorScale->axis()->setNumberFormat("gbc");
    colorScale->axis()->setNumberPrecision(0);
    colorScale->axis()->setRangeLower(1);

    ui->plot->replot();
    ui->plot->updateGeometry();
  } else {
    for (int i=0; i < ui->plot->plotLayout()->elementCount(); i++) {
      if (QCPColorScale *le = qobject_cast<QCPColorScale*>(ui->plot->plotLayout()->elementAt(i)))
        ui->plot->plotLayout()->remove(le);
    }
    ui->plot->plotLayout()->simplify();
    ui->plot->updateGeometry();
  }
}

void WidgetPlot2D::set_scale_type(QString sct) {
  this->setCursor(Qt::WaitCursor);
  current_scale_type_ = sct;
  colorMap->setDataScaleType(scale_types_[current_scale_type_]);
  colorMap->rescaleDataRange(true);
  ui->plot->replot();
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
    QFileInfo file(fileName);
    if (file.suffix() == "png")
      ui->plot->savePng(fileName,0,0,1,100);
    else if (file.suffix() == "jpg")
      ui->plot->saveJpg(fileName,0,0,1,100);
    else if (file.suffix() == "bmp")
      ui->plot->saveBmp(fileName);
    else if (file.suffix() == "pdf")
      ui->plot->savePdf(fileName, true);
  }
}

void WidgetPlot2D::setColorScheme(QColor fore, QColor back, QColor grid1, QColor grid2)
{
  ui->plot->xAxis->setBasePen(QPen(fore, 1));
  ui->plot->yAxis->setBasePen(QPen(fore, 1));
  ui->plot->xAxis->setTickPen(QPen(fore, 1));
  ui->plot->yAxis->setTickPen(QPen(fore, 1));
  ui->plot->xAxis->setSubTickPen(QPen(fore, 1));
  ui->plot->yAxis->setSubTickPen(QPen(fore, 1));
  ui->plot->xAxis->setTickLabelColor(fore);
  ui->plot->yAxis->setTickLabelColor(fore);
  ui->plot->xAxis->setLabelColor(fore);
  ui->plot->yAxis->setLabelColor(fore);
  ui->plot->xAxis->grid()->setPen(QPen(grid1, 1, Qt::DotLine));
  ui->plot->yAxis->grid()->setPen(QPen(grid1, 1, Qt::DotLine));
  ui->plot->xAxis->grid()->setSubGridPen(QPen(grid2, 1, Qt::DotLine));
  ui->plot->yAxis->grid()->setSubGridPen(QPen(grid2, 1, Qt::DotLine));
  ui->plot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
  ui->plot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
  ui->plot->setBackground(QBrush(back));
}
