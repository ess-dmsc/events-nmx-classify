/********************************************************************************
** Form generated from reading UI file 'Analyzer.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ANALYZER_H
#define UI_ANALYZER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qp_2d.h"
#include "qp_multi1d.h"

QT_BEGIN_NAMESPACE

class Ui_Analyzer
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_9;
    QComboBox *comboWeightsY;
    QLabel *label_10;
    QSpinBox *spinMinY;
    QLabel *label_12;
    QSpinBox *spinMaxY;
    QLabel *labelY;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_2;
    QPlot::Plot2D *plot2D;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QPlot::Multi1D *plotHistogram;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *pushAddBox;
    QPushButton *pushRemoveBox;
    QLabel *label_11;
    QTableView *tableBoxes;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_14;
    QComboBox *comboWeightsX;
    QLabel *label_15;
    QSpinBox *spinMinX;
    QLabel *label_16;
    QSpinBox *spinMaxX;
    QLabel *labelX;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_7;
    QComboBox *comboWeightsZ;
    QLabel *label_4;
    QSpinBox *spinMinZ;
    QLabel *label_5;
    QSpinBox *spinMaxZ;
    QLabel *labelZ;

    void setupUi(QWidget *Analyzer)
    {
        if (Analyzer->objectName().isEmpty())
            Analyzer->setObjectName(QStringLiteral("Analyzer"));
        Analyzer->resize(1459, 952);
        QIcon icon;
        icon.addFile(QStringLiteral(":/new/icons/qpx.xpm"), QSize(), QIcon::Normal, QIcon::Off);
        Analyzer->setWindowIcon(icon);
        gridLayout = new QGridLayout(Analyzer);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        label_9 = new QLabel(Analyzer);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setMinimumSize(QSize(30, 0));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_9);

        comboWeightsY = new QComboBox(Analyzer);
        comboWeightsY->setObjectName(QStringLiteral("comboWeightsY"));
        comboWeightsY->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        horizontalLayout_5->addWidget(comboWeightsY);

        label_10 = new QLabel(Analyzer);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setMinimumSize(QSize(60, 0));
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_10);

        spinMinY = new QSpinBox(Analyzer);
        spinMinY->setObjectName(QStringLiteral("spinMinY"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(spinMinY->sizePolicy().hasHeightForWidth());
        spinMinY->setSizePolicy(sizePolicy);
        spinMinY->setMinimum(-1000000);
        spinMinY->setMaximum(1000000);
        spinMinY->setSingleStep(1);

        horizontalLayout_5->addWidget(spinMinY);

        label_12 = new QLabel(Analyzer);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setMinimumSize(QSize(60, 0));
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_12);

        spinMaxY = new QSpinBox(Analyzer);
        spinMaxY->setObjectName(QStringLiteral("spinMaxY"));
        sizePolicy.setHeightForWidth(spinMaxY->sizePolicy().hasHeightForWidth());
        spinMaxY->setSizePolicy(sizePolicy);
        spinMaxY->setMinimum(-1000000);
        spinMaxY->setMaximum(1000000);
        spinMaxY->setSingleStep(1);
        spinMaxY->setValue(1000000);

        horizontalLayout_5->addWidget(spinMaxY);

        labelY = new QLabel(Analyzer);
        labelY->setObjectName(QStringLiteral("labelY"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(labelY->sizePolicy().hasHeightForWidth());
        labelY->setSizePolicy(sizePolicy1);
        labelY->setMinimumSize(QSize(0, 0));
        labelY->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(labelY);


        gridLayout->addLayout(horizontalLayout_5, 1, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        plot2D = new QPlot::Plot2D(Analyzer);
        plot2D->setObjectName(QStringLiteral("plot2D"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(plot2D->sizePolicy().hasHeightForWidth());
        plot2D->setSizePolicy(sizePolicy2);

        verticalLayout_2->addWidget(plot2D);


        horizontalLayout_2->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(Analyzer);
        label->setObjectName(QStringLiteral("label"));
        QFont font;
        font.setPointSize(11);
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);
        label->setFrameShape(QFrame::StyledPanel);
        label->setFrameShadow(QFrame::Sunken);
        label->setLineWidth(2);
        label->setMidLineWidth(0);

        verticalLayout->addWidget(label);

        plotHistogram = new QPlot::Multi1D(Analyzer);
        plotHistogram->setObjectName(QStringLiteral("plotHistogram"));
        sizePolicy2.setHeightForWidth(plotHistogram->sizePolicy().hasHeightForWidth());
        plotHistogram->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(plotHistogram);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        pushAddBox = new QPushButton(Analyzer);
        pushAddBox->setObjectName(QStringLiteral("pushAddBox"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/oxy/16/edit_add.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushAddBox->setIcon(icon1);

        horizontalLayout_4->addWidget(pushAddBox);

        pushRemoveBox = new QPushButton(Analyzer);
        pushRemoveBox->setObjectName(QStringLiteral("pushRemoveBox"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/oxy/16/editdelete.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushRemoveBox->setIcon(icon2);

        horizontalLayout_4->addWidget(pushRemoveBox);

        label_11 = new QLabel(Analyzer);
        label_11->setObjectName(QStringLiteral("label_11"));
        sizePolicy1.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy1);
        label_11->setMinimumSize(QSize(0, 0));
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label_11);


        verticalLayout->addLayout(horizontalLayout_4);

        tableBoxes = new QTableView(Analyzer);
        tableBoxes->setObjectName(QStringLiteral("tableBoxes"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(tableBoxes->sizePolicy().hasHeightForWidth());
        tableBoxes->setSizePolicy(sizePolicy3);

        verticalLayout->addWidget(tableBoxes);


        horizontalLayout_2->addLayout(verticalLayout);


        gridLayout->addLayout(horizontalLayout_2, 3, 0, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        label_14 = new QLabel(Analyzer);
        label_14->setObjectName(QStringLiteral("label_14"));
        label_14->setMinimumSize(QSize(30, 0));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_14);

        comboWeightsX = new QComboBox(Analyzer);
        comboWeightsX->setObjectName(QStringLiteral("comboWeightsX"));
        comboWeightsX->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        horizontalLayout_6->addWidget(comboWeightsX);

        label_15 = new QLabel(Analyzer);
        label_15->setObjectName(QStringLiteral("label_15"));
        label_15->setMinimumSize(QSize(60, 0));
        label_15->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_15);

        spinMinX = new QSpinBox(Analyzer);
        spinMinX->setObjectName(QStringLiteral("spinMinX"));
        sizePolicy.setHeightForWidth(spinMinX->sizePolicy().hasHeightForWidth());
        spinMinX->setSizePolicy(sizePolicy);
        spinMinX->setMinimum(-1000000);
        spinMinX->setMaximum(1000000);
        spinMinX->setSingleStep(1);

        horizontalLayout_6->addWidget(spinMinX);

        label_16 = new QLabel(Analyzer);
        label_16->setObjectName(QStringLiteral("label_16"));
        label_16->setMinimumSize(QSize(60, 0));
        label_16->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_16);

        spinMaxX = new QSpinBox(Analyzer);
        spinMaxX->setObjectName(QStringLiteral("spinMaxX"));
        sizePolicy.setHeightForWidth(spinMaxX->sizePolicy().hasHeightForWidth());
        spinMaxX->setSizePolicy(sizePolicy);
        spinMaxX->setMinimum(-1000000);
        spinMaxX->setMaximum(1000000);
        spinMaxX->setSingleStep(1);
        spinMaxX->setValue(1000000);

        horizontalLayout_6->addWidget(spinMaxX);

        labelX = new QLabel(Analyzer);
        labelX->setObjectName(QStringLiteral("labelX"));
        sizePolicy1.setHeightForWidth(labelX->sizePolicy().hasHeightForWidth());
        labelX->setSizePolicy(sizePolicy1);
        labelX->setMinimumSize(QSize(0, 0));
        labelX->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(labelX);


        gridLayout->addLayout(horizontalLayout_6, 0, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label_7 = new QLabel(Analyzer);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setMinimumSize(QSize(30, 0));
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_7);

        comboWeightsZ = new QComboBox(Analyzer);
        comboWeightsZ->setObjectName(QStringLiteral("comboWeightsZ"));
        comboWeightsZ->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        horizontalLayout_3->addWidget(comboWeightsZ);

        label_4 = new QLabel(Analyzer);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setMinimumSize(QSize(60, 0));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_4);

        spinMinZ = new QSpinBox(Analyzer);
        spinMinZ->setObjectName(QStringLiteral("spinMinZ"));
        sizePolicy.setHeightForWidth(spinMinZ->sizePolicy().hasHeightForWidth());
        spinMinZ->setSizePolicy(sizePolicy);
        spinMinZ->setMinimum(-1000000);
        spinMinZ->setMaximum(1000000);
        spinMinZ->setSingleStep(1);

        horizontalLayout_3->addWidget(spinMinZ);

        label_5 = new QLabel(Analyzer);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setMinimumSize(QSize(60, 0));
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_5);

        spinMaxZ = new QSpinBox(Analyzer);
        spinMaxZ->setObjectName(QStringLiteral("spinMaxZ"));
        sizePolicy.setHeightForWidth(spinMaxZ->sizePolicy().hasHeightForWidth());
        spinMaxZ->setSizePolicy(sizePolicy);
        spinMaxZ->setMinimum(-1000000);
        spinMaxZ->setMaximum(1000000);
        spinMaxZ->setSingleStep(1);
        spinMaxZ->setValue(1000000);

        horizontalLayout_3->addWidget(spinMaxZ);

        labelZ = new QLabel(Analyzer);
        labelZ->setObjectName(QStringLiteral("labelZ"));
        sizePolicy1.setHeightForWidth(labelZ->sizePolicy().hasHeightForWidth());
        labelZ->setSizePolicy(sizePolicy1);
        labelZ->setMinimumSize(QSize(0, 0));
        labelZ->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(labelZ);


        gridLayout->addLayout(horizontalLayout_3, 2, 0, 1, 1);


        retranslateUi(Analyzer);

        QMetaObject::connectSlotsByName(Analyzer);
    } // setupUi

    void retranslateUi(QWidget *Analyzer)
    {
        Analyzer->setWindowTitle(QApplication::translate("Analyzer", "Analyzer", 0));
        label_9->setText(QApplication::translate("Analyzer", "Y:", 0));
        label_10->setText(QApplication::translate("Analyzer", "Y min", 0));
        label_12->setText(QApplication::translate("Analyzer", "Y max", 0));
        labelY->setText(QString());
        label->setText(QApplication::translate("Analyzer", "Z projection (Events tab shows only those in selected Z range)", 0));
        pushAddBox->setText(QString());
        pushRemoveBox->setText(QString());
        label_11->setText(QString());
        label_14->setText(QApplication::translate("Analyzer", "X:", 0));
        label_15->setText(QApplication::translate("Analyzer", "X min", 0));
        label_16->setText(QApplication::translate("Analyzer", "X max", 0));
        labelX->setText(QString());
        label_7->setText(QApplication::translate("Analyzer", "Z:", 0));
        label_4->setText(QApplication::translate("Analyzer", "Z min", 0));
        label_5->setText(QApplication::translate("Analyzer", "Z max", 0));
        labelZ->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Analyzer: public Ui_Analyzer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ANALYZER_H
