#include "testwindow.h"
#include "ui_testwindow.h"
#include<QFileDialog>
#include<opencv2/highgui.hpp>
#include<opencv2/core.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/features2d/features2d.hpp>
#include<opencv2/xfeatures2d.hpp>
#include<vector>
#include<iostream>
#include<algorithm>
#include <numeric>      // std::iota
#include <functional>   // std::bind
using namespace std::placeholders;

TestWindow::TestWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestWindow)
{
    ui->setupUi(this);
}

TestWindow::~TestWindow()
{
    delete ui;
}
/////////////////////////////////// Load Vocabulary File /////////////////////////////
void TestWindow::on_pushButton_2_clicked()
{
    QString strFileName= QFileDialog::getOpenFileName();

    if(strFileName== ""){
        ui->lblVocabulary->setText("File not chosen");
        return;
    }

    FileStorage fs;
    fs.open(strFileName.toStdString(), FileStorage::READ);

    fs["vocabulary"]>>vocabulary;

    if(vocabulary.empty()){
        ui->lblVocabulary->setText("Error: Could not read from vocabulary file");
        return;
    }
    ui->lblVocabulary->setText(strFileName);

    fs.release();
}

/////////////////////////////////// Histogram Database Loading /////////////////////////////
void TestWindow::on_pushButton_3_clicked()
{
    QString strFileName= QFileDialog::getOpenFileName();
    cout<<strFileName.toStdString()<<endl;

    if(strFileName== ""){
        cout<<"File not chosen"<<endl;
        ui->lblHistDb->setText("File not chosen");
        return;
    }

    FileStorage fs;
    fs.open(strFileName.toStdString(), FileStorage::READ);


    if (!fs.isOpened())
    {
        cout << "Failed to open " << strFileName.toStdString() << endl;
        return;
    }

    FileNode fn = fs["histDatabase"];
    if (fn.empty()){
        cout<<"Empty filenode";
        return;
        }


    FileNodeIterator current = fn.begin(), it_end = fn.end(); // Go through the node

    for (; current != it_end; ++current)
    {
        vector<int> tmp;
        FileNode item = *current;
        item >> tmp;
        histDatabase.push_back(tmp);
    }

    cout<<"histDatabase retrieved"<<endl;

    FileNode fni = fs["imgIdDatabase"];
    if (fni.empty()){
            return;
        }

    FileNodeIterator currenti = fni.begin(), it_endi = fni.end(); // Go through the node

    for (; currenti != it_endi; ++currenti)
    {
        string tmpi;
        FileNode itemi = *currenti;
        itemi >> tmpi;
        imgIdDatabase.push_back(tmpi);
    }
    cout<<"imgIdDatabase retrieved"<<endl;

    if(histDatabase.empty()||(imgIdDatabase.empty())){
        ui->lblHistDb->setText("Error: Could not read from histDatabase/imgIdDatabase file");
        return;
    }
    ui->lblHistDb->setText(strFileName);

    fs.release();
}

/////////////////////////////////// Load Query Image /////////////////////////////
void TestWindow::on_pushButton_clicked()
{
    QString strFileName= QFileDialog::getOpenFileName();

    if(strFileName== ""){
        ui->lblQueryImg->setText("File not chosen");
        return;
    }
    queryImg=cv::imread(strFileName.toStdString());

    if(queryImg.empty()){
        ui->lblQueryImg->setText("Error: Image not read from file");
        return;
    }
    ui->lblQueryImg->setText(strFileName);

    QImage qQueryImg=matToQImage(queryImg);
    ui->QueryImage->setPixmap(QPixmap::fromImage(qQueryImg));
}

/////////////////////////////////// Compare /////////////////////////////
/// \brief compare
/// \param a
/// \param b
/// \param data
/// \return
///
bool compareX(int a, int b, vector<double> &data)
{
    return data[a]<data[b];
}

/////////////////////////////////// Identify writers /////////////////////////////
void TestWindow::on_pushButton_4_clicked()
{
    vector<cv::KeyPoint> keypoints;
    Mat matGrayscale;
    Mat descriptor;
    Mat descriptorFM;
    std::vector<std::vector<int> >* pointIdxsOfClusters= new std::vector<std::vector<int> >;
    vector<int> imgHist;

    //create Sift feature point detector and feature extractor
    Ptr<FeatureDetector> detectorFM=xfeatures2d::SiftFeatureDetector::create();
    Ptr<DescriptorExtractor> extractorFM=xfeatures2d::SiftDescriptorExtractor::create();

    //create a nearest neighbor matcher
    Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher);
    //create BoF descriptor extractor
    BOWImgDescriptorExtractor bowDE(extractorFM,matcher);

    bowDE.setVocabulary(vocabulary);
    cv::cvtColor(queryImg,matGrayscale,CV_BGR2GRAY);
    detectorFM->detect(matGrayscale,keypoints);
    extractorFM->compute(matGrayscale, keypoints,descriptor);
    bowDE.compute(descriptor,descriptorFM,pointIdxsOfClusters);

    cout<<"Query Image histogram: "<<endl;

    //compute histogram for image
    for (std::vector<std::vector<int> >::iterator it = pointIdxsOfClusters->begin() ; it != pointIdxsOfClusters->end(); ++it){
        std::vector<int> row=*it;
        cout<<row.size()<<",";
        imgHist.push_back(row.size());
    }
    cout<<endl;

    //compare imgHist with histDatabase
    vector<double> distVector;
    int cnt=0;
    for (vector<vector<int> >::iterator it1 = histDatabase.begin() ; it1 != histDatabase.end(); ++it1){
        std::vector<int> dbHistVect=*it1;
        double dist=computeDist(imgHist,dbHistVect);
        distVector.push_back(dist);
        cnt++;
    }
    cout<<endl;

    //cout<<"Distance"<<std::min_element(distVector.begin(),distVector.end())<<endl;
    std::vector<double>::iterator result = std::min_element(distVector.begin(),distVector.end());
    int index=std::distance(distVector.begin(), result);
    std::cout << "min element at: " <<index <<endl;
    cout<<"Min distance file :"<<imgIdDatabase[index]<<endl;

    //release memory
    matGrayscale.release();
    keypoints.clear();
    descriptor.release();
    pointIdxsOfClusters->clear();
    imgHist.clear();
    distVector.clear();

    //Distance for all test dataset
    cout<<endl<<"Computing distance for all test dataset:"<<endl;

    Mat img;

    QDir dir("/home/student/Dataset/iam/test_data");

    if (!dir.exists())
        qWarning("Cannot find the example directory");
    else
    {
        cout<<"Test dir opened"<<endl;

        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        int progress=0;
        int success=0;
        int idx[816];

        //Loop through all files
        foreach(QString dirFile, dir.entryList())
        {
             cout<<"Progress: "<<progress<<endl;
             QString filenameWithPath=dir.absolutePath()+"/"+dirFile;

             img=cv::imread(filenameWithPath.toStdString());
             cout<<"Test Image"<<dirFile.toStdString()<<" loaded "<<endl;

             cv::cvtColor(img,matGrayscale,CV_BGR2GRAY);
             detectorFM->detect(matGrayscale,keypoints);
             extractorFM->compute(matGrayscale, keypoints,descriptor);
             bowDE.compute(descriptor,descriptorFM,pointIdxsOfClusters);

             //cout<<"Computing Image histogram: ....... ";

             //compute histogram for image
             std::vector<int> row;
             for (std::vector<std::vector<int> >::iterator it2 = pointIdxsOfClusters->begin() ; it2 != pointIdxsOfClusters->end(); ++it2){
                 row=*it2;
                 imgHist.push_back(row.size());
             }
            //cout<<"DONE"<<endl;

            //Compute distance with histDatabase
            cnt=0;
            std::vector<int> dbHistVect;
            double dist;

            //cout<<"Computing distance from histogram database...";

            for (vector<vector<int> >::iterator it3 = histDatabase.begin() ; it3 != histDatabase.end(); ++it3){
                dbHistVect=*it3;
                dist=computeDist(imgHist,dbHistVect);
                distVector.push_back(dist);
                cnt++;
                dbHistVect.clear();
            }
            //cout<<"DONE"<<endl;

            std::iota(std::begin(idx), std::end(idx), 0);
            std::sort(std::begin(idx), std::end(idx), std::bind(compareX, _1, _2, distVector));

            std::cout << "Sorted first five forms: ";
            for (int i=0; i<5; i++){
                std::cout <<imgIdDatabase[idx[i]]<<":"<< distVector[idx[i]] << ","; std::cout << "\n";
                if(imgIdDatabase[idx[i]]==dirFile.remove(".png").toStdString())
                    success++;
            }

             //release memory
             img.release();
             matGrayscale.release();
             keypoints.clear();
             descriptor.release();
             pointIdxsOfClusters->clear();
             imgHist.clear();
             distVector.clear();

             progress++;
        }

        cout<<"All files tested"<<endl;
        cout<<"Success : "<<(double)success/progress<<endl;
    }

}


double TestWindow::computeDist(vector<int> queryImgHist,vector<int>dbImgHist){

    int q,d,sum=0;

    for (int i=0; i<queryImgHist.size();i++){
        q=queryImgHist[i];
        d=dbImgHist[i];
        if((d+q)!=0){
            sum+=pow((d-q),2)/(d+q);
        }
       // cout<<QString::number(q).toStdString()<<" , "<<QString::number(d).toStdString()<<" , "<<QString::number(q).toStdString()<<" , "<<QString::number(pow((d-q),2)).toStdString()<<endl;
    }
    return sqrt(sum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief OriginalImg::matToQImage
/// \param mat
/// \return image converted from cv::Mat to QImage format
////////////////////////////////////////////////////////////////////////////////////////////////////////////
QImage TestWindow::matToQImage(cv::Mat mat) {
    if(mat.channels() == 1) {                   // if grayscale image
        return QImage((uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);     // declare and return a QImage
    }
    else if(mat.channels() == 3) {            // if 3 channel color image
        cv::cvtColor(mat, mat, CV_BGR2RGB);     // invert BGR to RGB
        return QImage((uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);       // declare and return a QImage
    }
    else {
        ui->QueryImage->setText("in openCVMatToQImage, image was not 1 channel or 3 channel, should never get here");
    }

    return QImage();        // return a blank QImage if the above did not work
}
