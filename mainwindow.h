#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<opencv2/highgui.hpp>
#include<opencv2/core.hpp>
#include<opencv2/imgproc.hpp>

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();


    //void on_CreateHistogramDatabase_clicked();

   // void on_pushButton_2_clicked();

    void on_SelectVocabulary_clicked();

    void on_CreateHistDB_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    cv::Mat featuresUnclustered;
    cv::Mat vocabulary;
    QStringList strVocabularyFilelist;
    cv::Mat cropImage(cv::Mat inputImage);
    void preprocessIAM();
    vector<vector<float> > kmeansClusterMultiDim(int k);
    double computeDist(vector<float> vec1,vector<float> vec2);
    vector<float> calcAvgVec(vector<vector<float> > obj);
};

#endif // MAINWINDOW_H
