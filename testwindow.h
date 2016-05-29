#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QDialog>
#include<opencv2/highgui.hpp>
#include<opencv2/core.hpp>
#include<opencv2/imgproc.hpp>
#include<iostream>

using namespace cv;
using namespace std;

namespace Ui {
class TestWindow;
}

class TestWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TestWindow(QWidget *parent = 0);
    ~TestWindow();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::TestWindow *ui;
    cv::Mat vocabulary;
    vector<vector<int> > histDatabase;
    vector<string> imgIdDatabase;
    Mat queryImg;
    QString q_clust_center;
    //QImage matToQImage(cv::Mat mat);
    double computeDist(vector<int>,vector<int>);
    double computeDistChi(vector<int> ,vector<int>dbImgHist);

};

#endif // TESTWINDOW_H
