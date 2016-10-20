#ifndef QP_MULTI1D_H
#define QP_MULTI1D_H

#include <QWidget>
#include "qp_generic.h"
#include "qp_appearance.h"
#include "qp_entry2d.h"

namespace QPlot
{

struct Marker1D
{
  Marker1D() {}
  Marker1D(double p, Appearance app = Appearance(), bool vis = true)
    : pos(p), visible(vis), appearance(app) {}

  bool operator!= (const Marker1D& other) const { return (!operator==(other)); }
  bool operator== (const Marker1D& other) const { return (pos == other.pos); }

  bool visible {false};
  double pos {0};
  Appearance appearance;
};


class Multi1D : public GenericPlot
{
  Q_OBJECT

public:
  explicit Multi1D(QWidget *parent = 0);

  void clearPrimary() override;
  void clearExtras() override;
  void replotExtras() override;

  void addGraph(HistoData, Appearance);
  void setAxisLabels(QString x, QString y);
  void setTitle(QString title);

  void setMarkers(const std::list<Marker1D>&);
  std::set<double> selectedMarkers();
  void setHighlight(Marker1D, Marker1D);

  void tightenX();
  void setScaleType(QString) override;

public slots:
  void zoomOut() Q_DECL_OVERRIDE;

signals:
  void clickedLeft(double);
  void clickedRight(double);

protected slots:
  void mousePressed(QMouseEvent*);
  void mouseReleased(QMouseEvent*);

  void adjustY();

protected:
  void mouseClicked(double x, double y, QMouseEvent *event) override;

  QString title_text_;
  std::list<Marker1D> my_markers_;
  std::vector<Marker1D> highlight_;

  QCPGraphDataContainer aggregate_;
  QCPRange getDomain();
  QCPRange getRange(QCPRange domain = QCPRange());

  void plotMarkers();
  void plotHighlight();
  void plotTitle();
};

}

#endif
