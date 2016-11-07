#ifndef QP_PLOT2D_H
#define QP_PLOT2D_H

#include "qp_generic.h"
#include "qp_appearance.h"
#include "histogram.h"

namespace QPlot
{

struct MarkerBox2D
{
  bool operator== (const MarkerBox2D& other) const
  {
    if (x1 != other.x1) return false;
    if (x2 != other.x2) return false;
    if (y1 != other.y1) return false;
    if (y2 != other.y2) return false;
    return true;
  }

  bool operator!= (const MarkerBox2D& other) const
    { return !operator==(other); }

  bool selected {false};
  bool selectable {false};
  double x1, x2, y1, y2;
  QString label;
  QColor border;
  QColor fill;
};


class Plot2D : public GenericPlot
{
  Q_OBJECT

public:
  explicit Plot2D(QWidget *parent = 0);

  void clearPrimary() override;
  void clearExtras() override;
  void replotExtras() override;

  void updatePlot(uint64_t sizex, uint64_t sizey, const HistMap2D &spectrum_data);
  void updatePlot(uint64_t sizex, uint64_t sizey, const HistList2D &spectrum_data);

  void setAxes(QString xlabel, double x1, double x2,
                QString ylabel, double y1, double y2,
                QString zlabel);
  void setOrientation(Qt::Orientation);

  void setBoxes(std::list<MarkerBox2D> boxes);
  std::list<MarkerBox2D> selectedBoxes();

public slots:
  void zoomOut() Q_DECL_OVERRIDE;

signals:
  void clickedPlot(double x, double y, Qt::MouseButton button);

protected:
  void mouseClicked(double x, double y, QMouseEvent* event) override;

  QCPColorMap *colorMap {new QCPColorMap(xAxis, yAxis)};
  std::list<MarkerBox2D> boxes_;

  void initializeGradients();
  void plotBoxes();
};

}

#endif
