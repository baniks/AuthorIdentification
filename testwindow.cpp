#include "testwindow.h"
#include "ui_testwindow.h"
#include<QFileDialog>
#include<QFile>
#include<QTextStream>
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

/////////////////////////////////// Load Histogram Database file /////////////////////////////

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

/////////////////////////////////// Identify Writer for Query Image /////////////////////////////

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

    ///////////////// Compute distance from Histogram database ///////////////////////

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

    //compute histogram for quez image
    for (std::vector<std::vector<int> >::iterator it = pointIdxsOfClusters->begin() ; it != pointIdxsOfClusters->end(); ++it){
        std::vector<int> row=*it;
        cout<<row.size()<<",";
        imgHist.push_back(row.size());
    }
    cout<<endl;

    //compare query imgHist with histDatabase
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

/////////////////////////////////// Identify writers for All Test Data /////////////////////////////

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

    ///////////////// Calculating Distance from Histogram Database for All Test Dataset ///////////////////

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
        int idx[816];
        QVector<QStringList> top5_match;
        QVector<QStringList> top3_match;
        QFile top5file("top5_600.txt");

        if (!top5file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream outTop5(&top5file);

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

             //compute histogram for image
             std::vector<int> row;
             for (std::vector<std::vector<int> >::iterator it2 = pointIdxsOfClusters->begin() ; it2 != pointIdxsOfClusters->end(); ++it2){
                 row=*it2;
                 imgHist.push_back(row.size());
             }

            //Compute distance with histDatabase
            int cnt=0;
            vector<double> distVector;
            std::vector<int> dbHistVect;
            double dist;

            for (vector<vector<int> >::iterator it3 = histDatabase.begin() ; it3 != histDatabase.end(); ++it3){
                dbHistVect=*it3;
                dist=computeDist(imgHist,dbHistVect);
                distVector.push_back(dist);
                cnt++;
                dbHistVect.clear();
            }

            //Sorting according to increasing distance
            std::iota(std::begin(idx), std::end(idx), 0);
            std::sort(std::begin(idx), std::end(idx), std::bind(compareX, _1, _2, distVector));

            std::cout << "Sorted first five forms: ";
            QStringList top5,top3;
            for (int i=0; i<5; i++){
                std::cout <<imgIdDatabase[idx[i]]<<":"<< distVector[idx[i]] << ","; std::cout << "\n";
                QString matchedImg=QString::fromStdString(imgIdDatabase[idx[i]]);
                top5.append(matchedImg);

            }

            top5_match.push_back(top5);

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

        ///////////////// Calculating Test Statistics ///////////////////////////

        cout<<"Calculating test statistics..."<<endl;
        int i=0, success=0;
        double success_rate;

        foreach(QString dirFile, dir.entryList())
        {
            QString filename=dirFile.remove(".png");
            QStringList top5=top5_match[i];

            //Open forms1.txt to find the Writer of filename
            QFile formFile("/home/student/Desktop/forms1.txt");
            if(!formFile.open(QIODevice::ReadOnly| QIODevice::Text))
            {
                cout<<"Could not open forms1.txt"<<endl;
                return;
            }

            QTextStream form(&formFile);
            QString writerid;

            //Find writer of the file
            while(!form.atEnd())
            {
                QString line=form.readLine();
                if(line.split(" ")[0]==filename)
                {
                    writerid=line.split(" ")[1];
                    break;
                }
            }

            formFile.close();

            //Open uniq_writer-img_list to find list of imgs for the writer
            QFile writerImgFile("/home/student/Desktop/uniq_writer_img_list.txt");
            if(!writerImgFile.open(QIODevice::ReadOnly| QIODevice::Text))
            {
                cout<<"Could not open uniq_writer_img_list.txt"<<endl;
                return;
            }

            cout<<"Img: "<<filename.toStdString()<<"|Writer Id: "<<writerid.toStdString()<<"|Program Match: "<<(top5.join(",")).toStdString()<<"|Actual Match: ";;
            outTop5<<filename<<"|"<<writerid<<"|"<<top5.join(",")<<"|";
            QTextStream writeImg(&writerImgFile);
            QStringList actualMatch;

            //Find imglist of writerid
            while(!writeImg.atEnd())
            {
                QString line=writeImg.readLine();
                if(line.split("|")[0]==writerid)
                {
                    QString tempimglist=line.split("|")[1];
                    cout<<tempimglist.toStdString();
                    outTop5<<tempimglist;
                    actualMatch=tempimglist.split(",");
                    break;
                }
            }

            writerImgFile.close();

            int flag=0;
            int actual_success=0;
            int actual_match=0;
            cout<<"|Test Stat: ";
            outTop5<<"|";

            //Compare each actualMatch with top5
            for (int j = 0; j < actualMatch.size(); ++j)
            {
                QString temp=actualMatch[j];
                cout<<temp.toStdString()<<" : "<<top5.indexOf(actualMatch[j])<<", ";
                outTop5<<temp<<" : "<<top5.indexOf(actualMatch[j])<<", ";

                if(top5.contains(temp))
                {
                    flag=1;
                    actual_success++;
                }
                actual_match++;
            }


            if(flag==1)
            {
                success++;
            }
            else
            {
                cout<<"......No Match";
                outTop5<<"|No Match";
            }
            outTop5<<"|"<<actual_success<<"|"<<actual_match;
            cout<<endl;
            outTop5<<endl;
            i++;

        }

        success_rate=(double)success/progress;
        cout<<"Success : "<<success_rate<<endl;
        outTop5<<"Success : "<<success_rate<<endl;
        top5file.close();
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
