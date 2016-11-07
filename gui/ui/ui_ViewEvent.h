/********************************************************************************
** Form generated from reading UI file 'ViewEvent.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIEWEVENT_H
#define UI_VIEWEVENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>
#include "ViewRecord.h"
#include "qp_multi1d.h"

QT_BEGIN_NAMESPACE

class Ui_ViewEvent
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_6;
    QLabel *labelOfTotal_5;
    QLabel *label_14;
    QLabel *label_16;
    QComboBox *comboPoint1y;
    QLabel *labelPoint1y;
    QHBoxLayout *horizontalLayout_4;
    QLabel *labelOfTotal_3;
    QLabel *label_7;
    QLabel *label_12;
    QComboBox *comboPoint1x;
    QLabel *labelPoint1x;
    QHBoxLayout *horizontalLayout_5;
    QLabel *labelOfTotal_4;
    QLabel *label_11;
    QLabel *label_13;
    QComboBox *comboPoint2x;
    QLabel *labelPoint2x;
    QFrame *line_2;
    QPlot::Multi1D *plotProjection;
    QFrame *line_4;
    QHBoxLayout *horizontalLayout;
    QSpinBox *spinEventIdx;
    QLabel *labelOfTotal;
    QFrame *line_3;
    QLabel *label_4;
    QComboBox *comboPlanes;
    QLabel *label_3;
    QComboBox *comboPlot;
    QLabel *label_2;
    QComboBox *comboOverlay;
    QLabel *label_8;
    QComboBox *comboProjection;
    QLabel *label;
    QHBoxLayout *horizontalLayout_2;
    ViewRecord *eventX;
    QFrame *line;
    ViewRecord *eventY;
    QTableWidget *tableValues;
    QHBoxLayout *horizontalLayout_7;
    QLabel *labelOfTotal_6;
    QLabel *label_15;
    QLabel *label_18;
    QComboBox *comboPoint2y;
    QLabel *labelPoint2y;

    void setupUi(QWidget *ViewEvent)
    {
        if (ViewEvent->objectName().isEmpty())
            ViewEvent->setObjectName(QStringLiteral("ViewEvent"));
        ViewEvent->resize(1504, 952);
        QIcon icon;
        icon.addFile(QStringLiteral(":/new/icons/qpx.xpm"), QSize(), QIcon::Normal, QIcon::Off);
        ViewEvent->setWindowIcon(icon);
        gridLayout = new QGridLayout(ViewEvent);
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        labelOfTotal_5 = new QLabel(ViewEvent);
        labelOfTotal_5->setObjectName(QStringLiteral("labelOfTotal_5"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(labelOfTotal_5->sizePolicy().hasHeightForWidth());
        labelOfTotal_5->setSizePolicy(sizePolicy);

        horizontalLayout_6->addWidget(labelOfTotal_5);

        label_14 = new QLabel(ViewEvent);
        label_14->setObjectName(QStringLiteral("label_14"));
        label_14->setMinimumSize(QSize(60, 0));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_14);

        label_16 = new QLabel(ViewEvent);
        label_16->setObjectName(QStringLiteral("label_16"));
        label_16->setMinimumSize(QSize(60, 0));
        label_16->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_16);

        comboPoint1y = new QComboBox(ViewEvent);
        comboPoint1y->setObjectName(QStringLiteral("comboPoint1y"));

        horizontalLayout_6->addWidget(comboPoint1y);

        labelPoint1y = new QLabel(ViewEvent);
        labelPoint1y->setObjectName(QStringLiteral("labelPoint1y"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(labelPoint1y->sizePolicy().hasHeightForWidth());
        labelPoint1y->setSizePolicy(sizePolicy1);

        horizontalLayout_6->addWidget(labelPoint1y);


        gridLayout->addLayout(horizontalLayout_6, 3, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        labelOfTotal_3 = new QLabel(ViewEvent);
        labelOfTotal_3->setObjectName(QStringLiteral("labelOfTotal_3"));
        sizePolicy.setHeightForWidth(labelOfTotal_3->sizePolicy().hasHeightForWidth());
        labelOfTotal_3->setSizePolicy(sizePolicy);

        horizontalLayout_4->addWidget(labelOfTotal_3);

        label_7 = new QLabel(ViewEvent);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setMinimumSize(QSize(60, 0));
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label_7);

        label_12 = new QLabel(ViewEvent);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setMinimumSize(QSize(60, 0));
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label_12);

        comboPoint1x = new QComboBox(ViewEvent);
        comboPoint1x->setObjectName(QStringLiteral("comboPoint1x"));

        horizontalLayout_4->addWidget(comboPoint1x);

        labelPoint1x = new QLabel(ViewEvent);
        labelPoint1x->setObjectName(QStringLiteral("labelPoint1x"));
        sizePolicy1.setHeightForWidth(labelPoint1x->sizePolicy().hasHeightForWidth());
        labelPoint1x->setSizePolicy(sizePolicy1);

        horizontalLayout_4->addWidget(labelPoint1x);


        gridLayout->addLayout(horizontalLayout_4, 2, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        labelOfTotal_4 = new QLabel(ViewEvent);
        labelOfTotal_4->setObjectName(QStringLiteral("labelOfTotal_4"));
        sizePolicy.setHeightForWidth(labelOfTotal_4->sizePolicy().hasHeightForWidth());
        labelOfTotal_4->setSizePolicy(sizePolicy);

        horizontalLayout_5->addWidget(labelOfTotal_4);

        label_11 = new QLabel(ViewEvent);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setMinimumSize(QSize(60, 0));
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_11);

        label_13 = new QLabel(ViewEvent);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setMinimumSize(QSize(60, 0));
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_13);

        comboPoint2x = new QComboBox(ViewEvent);
        comboPoint2x->setObjectName(QStringLiteral("comboPoint2x"));

        horizontalLayout_5->addWidget(comboPoint2x);

        labelPoint2x = new QLabel(ViewEvent);
        labelPoint2x->setObjectName(QStringLiteral("labelPoint2x"));
        sizePolicy1.setHeightForWidth(labelPoint2x->sizePolicy().hasHeightForWidth());
        labelPoint2x->setSizePolicy(sizePolicy1);

        horizontalLayout_5->addWidget(labelPoint2x);


        gridLayout->addLayout(horizontalLayout_5, 4, 0, 1, 1);

        line_2 = new QFrame(ViewEvent);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_2, 1, 0, 1, 1);

        plotProjection = new QPlot::Multi1D(ViewEvent);
        plotProjection->setObjectName(QStringLiteral("plotProjection"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(plotProjection->sizePolicy().hasHeightForWidth());
        plotProjection->setSizePolicy(sizePolicy2);
        plotProjection->setMaximumSize(QSize(16777215, 450));

        gridLayout->addWidget(plotProjection, 8, 0, 1, 1);

        line_4 = new QFrame(ViewEvent);
        line_4->setObjectName(QStringLiteral("line_4"));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_4, 6, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        spinEventIdx = new QSpinBox(ViewEvent);
        spinEventIdx->setObjectName(QStringLiteral("spinEventIdx"));
        spinEventIdx->setEnabled(false);
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(spinEventIdx->sizePolicy().hasHeightForWidth());
        spinEventIdx->setSizePolicy(sizePolicy3);
        spinEventIdx->setMinimumSize(QSize(0, 28));

        horizontalLayout->addWidget(spinEventIdx);

        labelOfTotal = new QLabel(ViewEvent);
        labelOfTotal->setObjectName(QStringLiteral("labelOfTotal"));
        sizePolicy.setHeightForWidth(labelOfTotal->sizePolicy().hasHeightForWidth());
        labelOfTotal->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(labelOfTotal);

        line_3 = new QFrame(ViewEvent);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_3);

        label_4 = new QLabel(ViewEvent);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setMinimumSize(QSize(70, 0));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_4);

        comboPlanes = new QComboBox(ViewEvent);
        comboPlanes->setObjectName(QStringLiteral("comboPlanes"));

        horizontalLayout->addWidget(comboPlanes);

        label_3 = new QLabel(ViewEvent);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setMinimumSize(QSize(70, 0));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_3);

        comboPlot = new QComboBox(ViewEvent);
        comboPlot->setObjectName(QStringLiteral("comboPlot"));

        horizontalLayout->addWidget(comboPlot);

        label_2 = new QLabel(ViewEvent);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setMinimumSize(QSize(80, 0));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_2);

        comboOverlay = new QComboBox(ViewEvent);
        comboOverlay->setObjectName(QStringLiteral("comboOverlay"));

        horizontalLayout->addWidget(comboOverlay);

        label_8 = new QLabel(ViewEvent);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setMinimumSize(QSize(80, 0));
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_8);

        comboProjection = new QComboBox(ViewEvent);
        comboProjection->setObjectName(QStringLiteral("comboProjection"));

        horizontalLayout->addWidget(comboProjection);

        label = new QLabel(ViewEvent);
        label->setObjectName(QStringLiteral("label"));
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(label);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        eventX = new ViewRecord(ViewEvent);
        eventX->setObjectName(QStringLiteral("eventX"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(eventX->sizePolicy().hasHeightForWidth());
        eventX->setSizePolicy(sizePolicy4);

        horizontalLayout_2->addWidget(eventX);

        line = new QFrame(ViewEvent);
        line->setObjectName(QStringLiteral("line"));
        line->setMinimumSize(QSize(6, 0));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout_2->addWidget(line);

        eventY = new ViewRecord(ViewEvent);
        eventY->setObjectName(QStringLiteral("eventY"));
        sizePolicy4.setHeightForWidth(eventY->sizePolicy().hasHeightForWidth());
        eventY->setSizePolicy(sizePolicy4);

        horizontalLayout_2->addWidget(eventY);

        tableValues = new QTableWidget(ViewEvent);
        tableValues->setObjectName(QStringLiteral("tableValues"));
        sizePolicy.setHeightForWidth(tableValues->sizePolicy().hasHeightForWidth());
        tableValues->setSizePolicy(sizePolicy);
        tableValues->setMinimumSize(QSize(350, 0));
        tableValues->setMaximumSize(QSize(350, 16777215));

        horizontalLayout_2->addWidget(tableValues);


        gridLayout->addLayout(horizontalLayout_2, 7, 0, 1, 1);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        labelOfTotal_6 = new QLabel(ViewEvent);
        labelOfTotal_6->setObjectName(QStringLiteral("labelOfTotal_6"));
        sizePolicy.setHeightForWidth(labelOfTotal_6->sizePolicy().hasHeightForWidth());
        labelOfTotal_6->setSizePolicy(sizePolicy);

        horizontalLayout_7->addWidget(labelOfTotal_6);

        label_15 = new QLabel(ViewEvent);
        label_15->setObjectName(QStringLiteral("label_15"));
        label_15->setMinimumSize(QSize(60, 0));
        label_15->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_15);

        label_18 = new QLabel(ViewEvent);
        label_18->setObjectName(QStringLiteral("label_18"));
        label_18->setMinimumSize(QSize(60, 0));
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_18);

        comboPoint2y = new QComboBox(ViewEvent);
        comboPoint2y->setObjectName(QStringLiteral("comboPoint2y"));

        horizontalLayout_7->addWidget(comboPoint2y);

        labelPoint2y = new QLabel(ViewEvent);
        labelPoint2y->setObjectName(QStringLiteral("labelPoint2y"));
        sizePolicy1.setHeightForWidth(labelPoint2y->sizePolicy().hasHeightForWidth());
        labelPoint2y->setSizePolicy(sizePolicy1);

        horizontalLayout_7->addWidget(labelPoint2y);


        gridLayout->addLayout(horizontalLayout_7, 5, 0, 1, 1);


        retranslateUi(ViewEvent);

        QMetaObject::connectSlotsByName(ViewEvent);
    } // setupUi

    void retranslateUi(QWidget *ViewEvent)
    {
        ViewEvent->setWindowTitle(QApplication::translate("ViewEvent", "Energy Calibration", 0));
        labelOfTotal_5->setText(QString());
        label_14->setText(QString());
        label_16->setText(QApplication::translate("ViewEvent", "Y:", 0));
        labelPoint1y->setText(QString());
        labelOfTotal_3->setText(QString());
        label_7->setText(QApplication::translate("ViewEvent", "Point1:", 0));
        label_12->setText(QApplication::translate("ViewEvent", "X:", 0));
        labelPoint1x->setText(QString());
        labelOfTotal_4->setText(QString());
        label_11->setText(QApplication::translate("ViewEvent", "Point2:", 0));
        label_13->setText(QApplication::translate("ViewEvent", "X:", 0));
        labelPoint2x->setText(QString());
        labelOfTotal->setText(QString());
        label_4->setText(QApplication::translate("ViewEvent", "Planes:", 0));
        label_3->setText(QApplication::translate("ViewEvent", "Plot:", 0));
        label_2->setText(QApplication::translate("ViewEvent", "Overlay:", 0));
        label_8->setText(QApplication::translate("ViewEvent", "Projection:", 0));
        label->setText(QString());
        labelOfTotal_6->setText(QString());
        label_15->setText(QString());
        label_18->setText(QApplication::translate("ViewEvent", "Y:", 0));
        labelPoint2y->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class ViewEvent: public Ui_ViewEvent {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIEWEVENT_H
