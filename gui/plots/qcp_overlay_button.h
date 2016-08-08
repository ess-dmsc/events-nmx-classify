#ifndef QCP_OVERLAY_BUTTON
#define QCP_OVERLAY_BUTTON


#include "qcustomplot.h"

class QCPOverlayButton : public QCPItemPixmap
{
    Q_OBJECT
public:
    explicit QCPOverlayButton(QCustomPlot *parentPlot,
                           QPixmap pixmap,
                           QString name,
                           QString tooltip,
                           int second_point = (Qt::AlignBottom | Qt::AlignRight)
                           );

  QString name() {return name_;}
//  void set_image(QPixmap img);

private slots:
    void showTip(QMouseEvent *event);

private:
    QString name_;
    QString tooltip_;

};

#endif
