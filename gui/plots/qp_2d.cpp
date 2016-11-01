#include "qp_2d.h"

namespace QPlot
{

Plot2D::Plot2D(QWidget *parent)
  : GenericPlot(parent)
{
  initializeGradients();

  colorMap->setTightBoundary(false);
  axisRect()->setupFullAxesBox();
  setInteractions(static_cast<QCP::Interactions>(QCP::iRangeDrag | QCP::iRangeZoom));
  setNoAntialiasingOnDrag(true);

  setGradient("Greys");
  setGridStyle("Grid + subgrid");
  setScaleType("Linear");
  setAlwaysSquare(true);
  setAntialiased(false);

  setVisibleOptions(ShowOptions::zoom | ShowOptions::save | ShowOptions::grid |
                    ShowOptions::scale | ShowOptions::gradients);
}

void Plot2D::initializeGradients()
{
  addStandardGradients();

  addCustomGradient("Blues", {"#ffffff","#deebf7","#9ecae1","#3182bd"});
  addCustomGradient("Greens", {"#ffffff","#e5f5e0","#a1d99b","#31a354"});
  addCustomGradient("Oranges", {"#ffffff","#fee6ce","#fdae6b","#e6550d"});
  addCustomGradient("Purples", {"#ffffff","#efedf5","#bcbddc","#756bb1"});
  addCustomGradient("Reds", {"#ffffff","#fee0d2","#fc9272","#de2d26"});
  addCustomGradient("Greys", {"#ffffff","#f0f0f0","#bdbdbd","#636363"});

  addCustomGradient("GnBu5", {"#ffffff","#f0f9e8","#bae4bc","#7bccc4","#43a2ca","#0868ac"});
  addCustomGradient("PuRd5", {"#ffffff","#f1eef6","#d7b5d8","#df65b0","#dd1c77","#980043"});
  addCustomGradient("RdPu5", {"#ffffff","#feebe2","#fbb4b9","#f768a1","#c51b8a","#7a0177"});
  addCustomGradient("YlGn5", {"#ffffff","#ffffcc","#c2e699","#78c679","#31a354","#006837"});
  addCustomGradient("YlGnBu5", {"#ffffff","#ffffcc","#a1dab4","#41b6c4","#2c7fb8","#253494"});

  addCustomGradient("Spectrum2", {"#ffffff","#0000ff","#00ffff","#00ff00","#ffff00","#ff0000","#000000"});
}

void Plot2D::setOrientation(Qt::Orientation o)
{
  yAxis->axisRect()->setRangeDrag(o);
  yAxis->axisRect()->setRangeZoom(o);
}

void Plot2D::setBoxes(std::list<MarkerBox2D> boxes)
{
  boxes_ = boxes;
}

std::list<MarkerBox2D> Plot2D::selectedBoxes()
{
  std::list<MarkerBox2D> selection;
  for (auto &q : selectedItems())
    if (QCPItemRect *b = qobject_cast<QCPItemRect*>(q))
    {
      MarkerBox2D box;
      box.x1 = b->property("chan_x").toDouble();
      box.y1 = b->property("chan_y").toDouble();
      selection.push_back(box);
    }
  return selection;
}


void Plot2D::clearPrimary()
{
  colorMap->data()->clear();
}

void Plot2D::clearExtras()
{
  boxes_.clear();
}

void Plot2D::replotExtras()
{
  clearItems();
  plotBoxes();
  plotButtons();
  replot();
}

void Plot2D::plotBoxes()
{
  int selectables = 0;
  for (auto &q : boxes_)
  {
    //    if (!q.visible)
    //      continue;
    QCPItemRect *box = new QCPItemRect(this);
    box->setSelectable(q.selectable);
    box->setPen(q.border);
//    box->setSelectedPen(pen);
    box->setBrush(QBrush(q.fill));
    box->setSelected(q.selected);
    QColor sel = box->selectedPen().color();
    box->setSelectedBrush(QBrush(QColor::fromHsv(sel.hsvHue(), sel.saturation(), sel.value(), sel.alpha() * 0.15)));

    box->setProperty("chan_x", q.x1);
    box->setProperty("chan_y", q.y1);
    box->topLeft->setCoords(q.x1, q.y1);
    box->bottomRight->setCoords(q.x2, q.y2);

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

      labelItem->setPositionAlignment(static_cast<Qt::AlignmentFlag>(Qt::AlignTop|Qt::AlignLeft));
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
    }
  }

  if (selectables)
  {
    setInteraction(QCP::iSelectItems, true);
    setInteraction(QCP::iMultiSelect, false);
  }
  else
  {
    setInteraction(QCP::iSelectItems, false);
    setInteraction(QCP::iMultiSelect, false);
  }
}


void Plot2D::updatePlot(uint64_t sizex, uint64_t sizey, const HistMap2D &spectrum_data)
{
  colorMap->data()->clear();
  setAlwaysSquare(sizex == sizey);
  if (sizex == sizey)
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
  else
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  if ((sizex > 0) && (sizey > 0) && (spectrum_data.size()))
  {
    colorMap->data()->setSize(sizex, sizey);
    for (auto it : spectrum_data)
      if ((it.first.x >= 0) && (it.first.y >= 0))
        colorMap->data()->setCell(it.first.x, it.first.y, it.second);
    setScaleType(scaleType());
    setGradient(gradient());
    rescaleAxes();
    updateGeometry();
  }
  replotExtras();
}

void Plot2D::updatePlot(uint64_t sizex, uint64_t sizey, const HistList2D &spectrum_data)
{
  colorMap->data()->clear();
  setAlwaysSquare(sizex == sizey);
  if (sizex == sizey)
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
  else
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  if ((sizex > 0) && (sizey > 0) && (spectrum_data.size()))
  {
    colorMap->data()->setSize(sizex, sizey);
    for (auto it : spectrum_data)
      if ((it.x >= 0) && (it.y >= 0))
        colorMap->data()->setCell(it.x, it.y, it.v);
    setScaleType(scaleType());
    setGradient(gradient());
    rescaleAxes();
    updateGeometry();
  }
  replotExtras();
}

void Plot2D::setAxes(QString xlabel, double x1, double x2,
                            QString ylabel, double y1, double y2,
                            QString zlabel)
{
  for (int i=0; i < plotLayout()->elementCount(); i++)
    if (QCPColorScale *le = qobject_cast<QCPColorScale*>(plotLayout()->elementAt(i)))
      le->axis()->setLabel(zlabel);

  colorMap->keyAxis()->setLabel(xlabel);
  colorMap->valueAxis()->setLabel(ylabel);
  colorMap->data()->setRange(QCPRange(x1, x2),
                             QCPRange(y1, y2));

  colorMap->keyAxis()->setNumberFormat("f");
  colorMap->keyAxis()->setNumberPrecision(0);
  colorMap->valueAxis()->setNumberFormat("f");
  colorMap->valueAxis()->setNumberPrecision(0);
  colorMap->setProperty("z_label", zlabel);

  rescaleAxes();
}

void Plot2D::mouseClicked(double x, double y, QMouseEvent *event)
{
  emit clickedPlot(x, y, event->button());
}


void Plot2D::zoomOut()
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



}
