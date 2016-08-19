#include "qcp_overlay_button.h"
#include "CustomLogger.h"
#include <QToolTip>

QCPOverlayButton::QCPOverlayButton(QCustomPlot *parentPlot,
                             QPixmap pixmap,
                             QString name,
                             QString tooltip,
                             int second_point)
  : QCPItemPixmap(parentPlot)
  , name_(name)
  , tooltip_(tooltip)
{
  setPixmap(pixmap);
  setSelectable(false);

  if (second_point == (Qt::AlignBottom | Qt::AlignRight)) {
//    DBG << "Alightning bottom right";
    bottomRight->setType(QCPItemPosition::ptAbsolute);
    bottomRight->setParentAnchor(topLeft);
    bottomRight->setCoords(pixmap.width(), pixmap.height());
  } else {
//    DBG << "Alightning top left";
    topLeft->setType(QCPItemPosition::ptAbsolute);
    topLeft->setParentAnchor(bottomRight);
    topLeft->setCoords(-pixmap.width(), -pixmap.height());
  }

  connect(parentPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(showTip(QMouseEvent*)));
}

void QCPOverlayButton::showTip(QMouseEvent *event)
{
  double x = event->pos().x();
  double y = event->pos().y();

  if (visible()
      && (left->pixelPoint().x() <= x)
      && (x <= right->pixelPoint().x())
      && (top->pixelPoint().y() <= y)
      && (y <= bottom->pixelPoint().y()))
  {
    QToolTip::showText(parentPlot()->mapToGlobal(event->pos()), tooltip_);
  }
}

//void QCPOverlayButton::set_image(QPixmap img)
//{
//  setPixmap(img);
//}

