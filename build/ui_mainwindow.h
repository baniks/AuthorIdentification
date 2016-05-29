/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QLabel *label_2;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *lblChosenDir;
    QPushButton *pushButton;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_2;
    QLabel *lblVocabulary;
    QPushButton *SelectVocabulary;
    QPushButton *pushButton_2;
    QPushButton *CreateHistDB;
    QFrame *line;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(622, 279);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(20, 0, 561, 31));
        label_2->setAlignment(Qt::AlignCenter);
        layoutWidget = new QWidget(centralWidget);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 50, 351, 32));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        lblChosenDir = new QLabel(layoutWidget);
        lblChosenDir->setObjectName(QStringLiteral("lblChosenDir"));
        lblChosenDir->setAutoFillBackground(true);
        lblChosenDir->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(lblChosenDir);

        pushButton = new QPushButton(layoutWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout->addWidget(pushButton);

        horizontalLayout->setStretch(0, 1);
        layoutWidget1 = new QWidget(centralWidget);
        layoutWidget1->setObjectName(QStringLiteral("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(20, 160, 351, 32));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        lblVocabulary = new QLabel(layoutWidget1);
        lblVocabulary->setObjectName(QStringLiteral("lblVocabulary"));
        lblVocabulary->setAutoFillBackground(true);
        lblVocabulary->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(lblVocabulary);

        SelectVocabulary = new QPushButton(layoutWidget1);
        SelectVocabulary->setObjectName(QStringLiteral("SelectVocabulary"));

        horizontalLayout_2->addWidget(SelectVocabulary);

        horizontalLayout_2->setStretch(0, 1);
        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setEnabled(false);
        pushButton_2->setGeometry(QRect(80, 90, 181, 32));
        CreateHistDB = new QPushButton(centralWidget);
        CreateHistDB->setObjectName(QStringLiteral("CreateHistDB"));
        CreateHistDB->setEnabled(false);
        CreateHistDB->setGeometry(QRect(80, 200, 201, 32));
        line = new QFrame(centralWidget);
        line->setObjectName(QStringLiteral("line"));
        line->setGeometry(QRect(20, 130, 561, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        MainWindow->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Training Window", 0));
        label_2->setText(QApplication::translate("MainWindow", "Author Identification - Training", 0));
        lblChosenDir->setText(QApplication::translate("MainWindow", "Select training images", 0));
        pushButton->setText(QApplication::translate("MainWindow", "Browse to Upload", 0));
        lblVocabulary->setText(QApplication::translate("MainWindow", "Select vocabulary file", 0));
        SelectVocabulary->setText(QApplication::translate("MainWindow", "Browse to Upload", 0));
        pushButton_2->setText(QApplication::translate("MainWindow", "Create Vocabulary", 0));
        CreateHistDB->setText(QApplication::translate("MainWindow", "Create Histogram Database", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
