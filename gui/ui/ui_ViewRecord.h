/********************************************************************************
** Form generated from reading UI file 'ViewRecord.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIEWRECORD_H
#define UI_VIEWRECORD_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include "qp_2d.h"

QT_BEGIN_NAMESPACE

class Ui_ViewRecord
{
public:
    QGridLayout *gridLayout;
    QLabel *labelTitle;
    QPlot::Plot2D *plotRecord;

    void setupUi(QWidget *ViewRecord)
    {
        if (ViewRecord->objectName().isEmpty())
            ViewRecord->setObjectName(QStringLiteral("ViewRecord"));
        ViewRecord->resize(1504, 851);
        QIcon icon;
        icon.addFile(QStringLiteral(":/new/icons/qpx.xpm"), QSize(), QIcon::Normal, QIcon::Off);
        ViewRecord->setWindowIcon(icon);
        gridLayout = new QGridLayout(ViewRecord);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setVerticalSpacing(6);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        labelTitle = new QLabel(ViewRecord);
        labelTitle->setObjectName(QStringLiteral("labelTitle"));
        QFont font;
        font.setPointSize(16);
        labelTitle->setFont(font);
        labelTitle->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(labelTitle, 1, 0, 1, 1);

        plotRecord = new QPlot::Plot2D(ViewRecord);
        plotRecord->setObjectName(QStringLiteral("plotRecord"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(plotRecord->sizePolicy().hasHeightForWidth());
        plotRecord->setSizePolicy(sizePolicy);

        gridLayout->addWidget(plotRecord, 2, 0, 1, 1);


        retranslateUi(ViewRecord);

        QMetaObject::connectSlotsByName(ViewRecord);
    } // setupUi

    void retranslateUi(QWidget *ViewRecord)
    {
        ViewRecord->setWindowTitle(QApplication::translate("ViewRecord", "Energy Calibration", 0));
        labelTitle->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class ViewRecord: public Ui_ViewRecord {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIEWRECORD_H
