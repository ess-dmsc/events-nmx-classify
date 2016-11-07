/********************************************************************************
** Form generated from reading UI file 'tpcc.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TPCC_H
#define UI_TPCC_H

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
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_tpcc
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *label_2;
    QTableWidget *tableParams;
    QTabWidget *tabWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushOpen;
    QFrame *line_2;
    QPushButton *pushDeleteGroup;
    QPushButton *pushNewGroup;
    QComboBox *comboGroup;
    QFrame *line;
    QPushButton *pushStart;
    QPushButton *pushStop;
    QProgressBar *progressBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *tpcc)
    {
        if (tpcc->objectName().isEmpty())
            tpcc->setObjectName(QStringLiteral("tpcc"));
        tpcc->resize(1582, 913);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/tpcc.xpm"), QSize(), QIcon::Normal, QIcon::Off);
        tpcc->setWindowIcon(icon);
        tpcc->setIconSize(QSize(18, 18));
        tpcc->setDocumentMode(false);
        centralWidget = new QWidget(tpcc);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(label_2);

        tableParams = new QTableWidget(centralWidget);
        tableParams->setObjectName(QStringLiteral("tableParams"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tableParams->sizePolicy().hasHeightForWidth());
        tableParams->setSizePolicy(sizePolicy1);
        tableParams->setMinimumSize(QSize(300, 0));
        tableParams->setMaximumSize(QSize(300, 16777215));

        verticalLayout->addWidget(tableParams);


        horizontalLayout_2->addLayout(verticalLayout);

        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy2);

        horizontalLayout_2->addWidget(tabWidget);


        gridLayout->addLayout(horizontalLayout_2, 1, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        pushOpen = new QPushButton(centralWidget);
        pushOpen->setObjectName(QStringLiteral("pushOpen"));
        pushOpen->setEnabled(true);
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(pushOpen->sizePolicy().hasHeightForWidth());
        pushOpen->setSizePolicy(sizePolicy3);
        pushOpen->setMinimumSize(QSize(35, 35));
        pushOpen->setMaximumSize(QSize(16777215, 35));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/oxy/16/document_open_folder.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushOpen->setIcon(icon1);
        pushOpen->setIconSize(QSize(22, 22));
        pushOpen->setFlat(true);

        horizontalLayout->addWidget(pushOpen);

        line_2 = new QFrame(centralWidget);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_2);

        pushDeleteGroup = new QPushButton(centralWidget);
        pushDeleteGroup->setObjectName(QStringLiteral("pushDeleteGroup"));
        pushDeleteGroup->setEnabled(true);
        sizePolicy3.setHeightForWidth(pushDeleteGroup->sizePolicy().hasHeightForWidth());
        pushDeleteGroup->setSizePolicy(sizePolicy3);
        pushDeleteGroup->setMinimumSize(QSize(35, 35));
        pushDeleteGroup->setMaximumSize(QSize(16777215, 35));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/oxy/16/editdelete.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushDeleteGroup->setIcon(icon2);
        pushDeleteGroup->setIconSize(QSize(22, 22));
        pushDeleteGroup->setFlat(true);

        horizontalLayout->addWidget(pushDeleteGroup);

        pushNewGroup = new QPushButton(centralWidget);
        pushNewGroup->setObjectName(QStringLiteral("pushNewGroup"));
        pushNewGroup->setEnabled(true);
        sizePolicy3.setHeightForWidth(pushNewGroup->sizePolicy().hasHeightForWidth());
        pushNewGroup->setSizePolicy(sizePolicy3);
        pushNewGroup->setMinimumSize(QSize(35, 35));
        pushNewGroup->setMaximumSize(QSize(16777215, 35));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/oxy/16/document_new.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushNewGroup->setIcon(icon3);
        pushNewGroup->setIconSize(QSize(22, 22));
        pushNewGroup->setFlat(true);

        horizontalLayout->addWidget(pushNewGroup);

        comboGroup = new QComboBox(centralWidget);
        comboGroup->setObjectName(QStringLiteral("comboGroup"));
        sizePolicy3.setHeightForWidth(comboGroup->sizePolicy().hasHeightForWidth());
        comboGroup->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(comboGroup);

        line = new QFrame(centralWidget);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line);

        pushStart = new QPushButton(centralWidget);
        pushStart->setObjectName(QStringLiteral("pushStart"));
        pushStart->setEnabled(true);
        sizePolicy3.setHeightForWidth(pushStart->sizePolicy().hasHeightForWidth());
        pushStart->setSizePolicy(sizePolicy3);
        pushStart->setMinimumSize(QSize(35, 35));
        pushStart->setMaximumSize(QSize(16777215, 35));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/icons/oxy/16/1rightarrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushStart->setIcon(icon4);
        pushStart->setIconSize(QSize(22, 22));
        pushStart->setFlat(true);

        horizontalLayout->addWidget(pushStart);

        pushStop = new QPushButton(centralWidget);
        pushStop->setObjectName(QStringLiteral("pushStop"));
        pushStop->setEnabled(false);
        sizePolicy3.setHeightForWidth(pushStop->sizePolicy().hasHeightForWidth());
        pushStop->setSizePolicy(sizePolicy3);
        pushStop->setMinimumSize(QSize(35, 35));
        pushStop->setMaximumSize(QSize(16777215, 35));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/icons/oxy/16/stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushStop->setIcon(icon5);
        pushStop->setIconSize(QSize(22, 22));
        pushStop->setFlat(true);

        horizontalLayout->addWidget(pushStop);

        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        QSizePolicy sizePolicy4(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(progressBar->sizePolicy().hasHeightForWidth());
        progressBar->setSizePolicy(sizePolicy4);
        progressBar->setValue(0);

        horizontalLayout->addWidget(progressBar);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

        tpcc->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(tpcc);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        tpcc->setStatusBar(statusBar);

        retranslateUi(tpcc);

        tabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(tpcc);
    } // setupUi

    void retranslateUi(QMainWindow *tpcc)
    {
        tpcc->setWindowTitle(QApplication::translate("tpcc", "TPC Classify", 0));
        label_2->setText(QApplication::translate("tpcc", "Analysis parameters", 0));
#ifndef QT_NO_TOOLTIP
        pushOpen->setToolTip(QApplication::translate("tpcc", "Start", 0));
#endif // QT_NO_TOOLTIP
        pushOpen->setText(QString());
#ifndef QT_NO_TOOLTIP
        pushDeleteGroup->setToolTip(QApplication::translate("tpcc", "Start", 0));
#endif // QT_NO_TOOLTIP
        pushDeleteGroup->setText(QString());
#ifndef QT_NO_TOOLTIP
        pushNewGroup->setToolTip(QApplication::translate("tpcc", "Start", 0));
#endif // QT_NO_TOOLTIP
        pushNewGroup->setText(QString());
#ifndef QT_NO_TOOLTIP
        pushStart->setToolTip(QApplication::translate("tpcc", "Start", 0));
#endif // QT_NO_TOOLTIP
        pushStart->setText(QString());
#ifndef QT_NO_TOOLTIP
        pushStop->setToolTip(QApplication::translate("tpcc", "Stop", 0));
#endif // QT_NO_TOOLTIP
        pushStop->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class tpcc: public Ui_tpcc {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TPCC_H
