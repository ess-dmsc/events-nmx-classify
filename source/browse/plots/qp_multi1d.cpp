#include "qp_multi1d.h"

namespace QPlot
{

Multi1D::Multi1D(QWidget *parent)
  : GenericPlot(parent)
{
  setInteraction(QCP::iSelectItems, true);
  setInteraction(QCP::iRangeDrag, true);
  yAxis->axisRect()->setRangeDrag(Qt::Horizontal);
  setInteraction(QCP::iRangeZoom, true);
  setInteraction(QCP::iMultiSelect, true);
  yAxis->setPadding(28);
  setNoAntialiasingOnDrag(true);

  connect(this, SIGNAL(beforeReplot()), this, SLOT(adjustY()));
  connect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleased(QMouseEvent*)));
  connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressed(QMouseEvent*)));

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
  connect(shortcut, SIGNAL(activated()), this, SLOT(zoomOut()));

  setScaleType("Logarithmic");
  setPlotStyle("Step center");
  setGridStyle("Grid + subgrid");

  setVisibleOptions(ShowOptions::zoom | ShowOptions::save |
                    ShowOptions::scale | ShowOptions::style |
                    ShowOptions::thickness | ShowOptions::title);

  replotExtras();
}

void Multi1D::clearPrimary()
{
  GenericPlot::clearGraphs();
  aggregate_.clear();
}

void Multi1D::clearExtras()
{
  my_markers_.clear();
  highlight_.clear();
  title_text_.clear();
}

void Multi1D::setAxisLabels(QString x, QString y)
{
  xAxis->setLabel(x);
  yAxis->setLabel(y);
}


void Multi1D::setTitle(QString title)
{
  title_text_ = title;
}

void Multi1D::setMarkers(const std::list<Marker1D>& markers)
{
  my_markers_ = markers;
}

void Multi1D::setHighlight(Marker1D a, Marker1D b)
{
  highlight_.resize(2);
  highlight_[0] = a;
  highlight_[1] = b;
}

std::set<double> Multi1D::selectedMarkers()
{
  std::set<double> selection;
  for (auto &q : selectedItems())
    if (QCPItemText *txt = qobject_cast<QCPItemText*>(q))
    {
      if (txt->property("position").isValid())
        selection.insert(txt->property("position").toDouble());
    }
    else if (QCPItemLine *line = qobject_cast<QCPItemLine*>(q))
    {
      if (line->property("position").isValid())
        selection.insert(line->property("position").toDouble());
    }

  return selection;
}


void Multi1D::addGraph(const HistMap1D &hist, Appearance appearance, QString name)
{
  if (hist.empty())
    return;

  GenericPlot::addGraph();
  int g = graphCount() - 1;
  graph(g)->setName(name);
  auto data = graph(g)->data();
  for (auto p :hist)
  {
    QCPGraphData point(p.first, p.second);
    data->add(point);
    aggregate_.add(point);
  }

  graph(g)->setPen(appearance.default_pen);
  setGraphStyle(graph(g));
  setGraphThickness(graph(g));
}

void Multi1D::addGraph(const HistList1D &hist, Appearance appearance, QString name)
{
  if (hist.empty())
    return;

  GenericPlot::addGraph();
  int g = graphCount() - 1;
  graph(g)->setName(name);
  auto data = graph(g)->data();
  for (auto p :hist)
  {
    QCPGraphData point(p.first, p.second);
    data->add(point);
    aggregate_.add(point);
  }

  graph(g)->setPen(appearance.default_pen);
  setGraphStyle(graph(g));
  setGraphThickness(graph(g));
}

QCPRange Multi1D::getDomain()
{
  QCP::SignDomain sd = QCP::sdBoth;
  if (scaleType() == "Logarithmic")
    sd = QCP::sdPositive;
  bool ok{false};
  return aggregate_.keyRange(ok, sd);
}

QCPRange Multi1D::getRange(QCPRange domain)
{
  bool log = (scaleType() == "Logarithmic");
  QCP::SignDomain sd = QCP::sdBoth;
  if (log)
    sd = QCP::sdPositive;
  bool ok{false};
  QCPRange range = aggregate_.valueRange(ok, sd, domain);
  if (ok)
  {
    range.upper = yAxis->pixelToCoord(yAxis->coordToPixel(range.upper) -
                                      ((!showTitle() || title_text_.isEmpty()) ? 5 : 25));
    double lower = yAxis->pixelToCoord(yAxis->coordToPixel(range.lower) + 5);
    if (log && (lower < 0))
      range.lower = 0;
    else
      range.lower = lower;
  }
  return range;
}

void Multi1D::tightenX()
{
  xAxis->setRange(getDomain());
}

void Multi1D::replotExtras()
{
  clearItems();
  plotMarkers();
  plotHighlight();
  plotTitle();
  plotButtons();
}

void Multi1D::plotMarkers()
{
  for (auto &q : my_markers_) {
    QCPItemTracer *top_crs = nullptr;
    if (q.visible)
    {
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

    if (top_crs != nullptr)
    {
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

      if (showMarkerLabels())
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
  }
}

void Multi1D::plotHighlight()
{
  if ((highlight_.size() == 2) && (highlight_[0].visible) && !aggregate_.isEmpty())
  {
    QCPRange range = getRange(getDomain());

    QCPItemRect *cprect = new QCPItemRect(this);

    cprect->topLeft->setCoords(highlight_[0].pos, range.upper);
    cprect->bottomRight->setCoords( highlight_[1].pos, range.lower);
    cprect->setPen(highlight_[0].appearance.default_pen);
    cprect->setBrush(QBrush(highlight_[1].appearance.default_pen.color()));
    cprect->setSelectable(false);
  }
}

void Multi1D::plotTitle()
{
  if (showTitle() && !title_text_.isEmpty())
  {
    QCPItemText *floatingText = new QCPItemText(this);
    floatingText->setPositionAlignment(static_cast<Qt::AlignmentFlag>(Qt::AlignTop|Qt::AlignHCenter));
    floatingText->position->setType(QCPItemPosition::ptAxisRectRatio);
    floatingText->position->setCoords(0.5, 0); // place position at center/top of axis rect
    floatingText->setText(title_text_);
    floatingText->setFont(QFont("Helvetica", 10));
    floatingText->setSelectable(false);
    floatingText->setColor(Qt::black);
  }
}

void Multi1D::mouseClicked(double x, double y, QMouseEvent* event)
{
  Q_UNUSED(y)
  if (event->button() == Qt::RightButton)
    emit clickedRight(x);
  else
    emit clickedLeft(x);
}

void Multi1D::zoomOut()
{
  xAxis->setRange(getDomain());
  adjustY();
  replot();
}

void Multi1D::mousePressed(QMouseEvent*)
{
  disconnect(this, SIGNAL(beforeReplot()), this, SLOT(adjustY()));
}

void Multi1D::mouseReleased(QMouseEvent*)
{
  connect(this, SIGNAL(beforeReplot()), this, SLOT(adjustY()));
  adjustY();
  replot();
}

void Multi1D::adjustY()
{
  if (aggregate_.isEmpty())
    rescaleAxes();
  else
    yAxis->setRange(getRange(xAxis->range()));
}

void Multi1D::setScaleType(QString type)
{
  bool diff = (type != scaleType());
  GenericPlot::setScaleType(type);
  if (diff)
  {
    replotExtras();
    replot();
  }
}


}
