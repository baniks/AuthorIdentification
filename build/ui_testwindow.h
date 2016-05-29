/********************************************************************************
** Form generated from reading UI file 'testwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTWINDOW_H
#define UI_TESTWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TestWindow
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QLabel *lblQueryImg;
    QWidget *layoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton_2;
    QLabel *lblVocabulary;
    QWidget *layoutWidget_3;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton_3;
    QLabel *lblHistDb;
    QPushButton *pushButton_4;

    void setupUi(QDialog *TestWindow)
    {
        if (TestWindow->objectName().isEmpty())
            TestWindow->setObjectName(QStringLiteral("TestWindow"));
        TestWindow->resize(631, 247);
        layoutWidget = new QWidget(TestWindow);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 110, 541, 41));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        pushButton = new QPushButton(layoutWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout->addWidget(pushButton);

        lblQueryImg = new QLabel(layoutWidget);
        lblQueryImg->setObjectName(QStringLiteral("lblQueryImg"));
        lblQueryImg->setAutoFillBackground(true);

        horizontalLayout->addWidget(lblQueryImg);

        horizontalLayout->setStretch(1, 1);
        layoutWidget_2 = new QWidget(TestWindow);
        layoutWidget_2->setObjectName(QStringLiteral("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(20, 10, 541, 41));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget_2);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        pushButton_2 = new QPushButton(layoutWidget_2);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));

        horizontalLayout_2->addWidget(pushButton_2);

        lblVocabulary = new QLabel(layoutWidget_2);
        lblVocabulary->setObjectName(QStringLiteral("lblVocabulary"));
        lblVocabulary->setAutoFillBackground(true);

        horizontalLayout_2->addWidget(lblVocabulary);

        horizontalLayout_2->setStretch(1, 1);
        layoutWidget_3 = new QWidget(TestWindow);
        layoutWidget_3->setObjectName(QStringLiteral("layoutWidget_3"));
        layoutWidget_3->setGeometry(QRect(20, 60, 541, 41));
        horizontalLayout_3 = new QHBoxLayout(layoutWidget_3);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        pushButton_3 = new QPushButton(layoutWidget_3);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));

        horizontalLayout_3->addWidget(pushButton_3);

        lblHistDb = new QLabel(layoutWidget_3);
        lblHistDb->setObjectName(QStringLiteral("lblHistDb"));
        lblHistDb->setAutoFillBackground(true);

        horizontalLayout_3->addWidget(lblHistDb);

        horizontalLayout_3->setStretch(1, 1);
        pushButton_4 = new QPushButton(TestWindow);
        pushButton_4->setObjectName(QStringLiteral("pushButton_4"));
        pushButton_4->setGeometry(QRect(20, 170, 291, 41));

        retranslateUi(TestWindow);

        QMetaObject::connectSlotsByName(TestWindow);
    } // setupUi

    void retranslateUi(QDialog *TestWindow)
    {
        TestWindow->setWindowTitle(QApplication::translate("TestWindow", "Test Dialog", 0));
        pushButton->setText(QApplication::translate("TestWindow", "Upload query image", 0));
        lblQueryImg->setText(QString());
        pushButton_2->setText(QApplication::translate("TestWindow", "Upload Vocabulary", 0));
        lblVocabulary->setText(QString());
        pushButton_3->setText(QApplication::translate("TestWindow", "Upload histogram database", 0));
        lblHistDb->setText(QString());
        pushButton_4->setText(QApplication::translate("TestWindow", "Calculate Test Statistics for All Test Data", 0));
    } // retranslateUi

};

namespace Ui {
    class TestWindow: public Ui_TestWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTWINDOW_H
