/***********************************************************************
* FILENAME :        testwindow.cpp
*
* DESCRIPTION :
*       This window is for test phase. It contains a simple UI which
*       gives options for either querying documents written by same author
*       for a single document or testing the same for all test images.
*
* SLOTS :
*       void    on_pushButton_2_clicked( )
*       void    on_pushButton_3_clicked()
*       void    on_pushButton_clicked()
*       void    on_pushButton_4_clicked()
*
* FUNCTIONS:
*       bool compareX(int , int , vector<double>& )
*       double computeDistChi(vector<int> ,vector<int> )
*       double computeDist(vector<int> ,vector<int> )
*       QImage matToQImage(cv::Mat mat)
*
************************************************************************/
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
#include <math.h>
using namespace std::placeholders;

extern QString training_path;
extern QString test_path;
extern QString out_dir;
extern QString form_file_path;
extern QString uniqWriterImgFile_path;

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


/***************************************************************************
* NAME :            TestWindow::on_pushButton_2_clicked
*
* DESCRIPTION :     This function loads the vocabulary file from disk.
*
* INPUTS :
*       PARAMETERS:
*           None
*       GLOBALS :
*           None
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  Prompts to select vocabulary file
*                   [2]  Loads vocabulary file to class member variable
*
****************************************************************************/

void TestWindow::on_pushButton_2_clicked()
{
    //Prompts to select vocabulary file
    QString strFileName= QFileDialog::getOpenFileName();

    if(strFileName== ""){
        ui->lblVocabulary->setText("File not chosen");
        return;
    }

    //Load vocabulary filecfrom disk to class member variable
    FileStorage fs;
    fs.open(strFileName.toStdString(), FileStorage::READ);

    fs["vocabulary"]>>vocabulary;

    if(vocabulary.empty()){
        ui->lblVocabulary->setText("Error: Could not read from vocabulary file");
        return;
    }
    ui->lblVocabulary->setText(strFileName);

    fs.release();

    //Derive and save chosen cluster center for future use
    QStringList pieces = strFileName.split( "/" );
    QString vocabFilenm = pieces.value( pieces.length() - 1 );
    vocabFilenm.remove(".yml");
    vocabFilenm.remove("dictionary_");

    q_clust_center=vocabFilenm;

}


/***************************************************************************
* NAME :            TestWindow::on_pushButton_3_clicked
*
* DESCRIPTION :     This function loads the vocabulary file from disk.
*
* INPUTS :
*       PARAMETERS:
*           None
*       GLOBALS :
*           None
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  Prompts to select histogram database file
*                   [2]  Loads histograms database from file
*                   [3]  Loads training image identifiers from file
*
****************************************************************************/

void TestWindow::on_pushButton_3_clicked()
{
    //Prompts to load histogram database file
    QString strFileName= QFileDialog::getOpenFileName();
    cout<<strFileName.toStdString()<<endl;

    if(strFileName== ""){
        cout<<"File not chosen"<<endl;
        ui->lblHistDb->setText("File not chosen");
        return;
    }

    //Loads histogram database file from disk
    FileStorage fs;
    fs.open(strFileName.toStdString(), FileStorage::READ);

    if (!fs.isOpened())
    {
        cout << "Failed to open " << strFileName.toStdString() << endl;
        return;
    }

    //Loads histDatabase from file to class member variable
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

    //Loads imgIdDatabase from file to class member variable
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


/*****************************************************************************
* NAME :            compareX
*
* DESCRIPTION :     This function is a helper function for comparing two
*                   vectors.
*
* INPUTS :
*       PARAMETERS:
*           int             a      Index
*           int             b      Index
*      vector<double> &     data   Vector
*       GLOBALS :
*           None
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  Compares the data of vector at the two input indexes.
*
*******************************************************************************/

bool compareX(int a, int b, vector<double> &data)
{
    return data[a]<data[b];
}


/*****************************************************************************
* NAME :            TestWindow::on_pushButton_clicked
*
* DESCRIPTION :     This function finds the documents written by the author
*                   of the query document.
*
* INPUTS :
*       PARAMETERS:
*           None
*       GLOBALS :
*           None
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  Prompts to select the query image.
*                   [2]  Display the query image.
*                   [3]  Feature extraction on query image
*                   [4]  Calculate distance from histogram database of training images
*                   [5]  Determine top 5 match
*                   [6]  Calculate success rate on top 5 statistics
*                   [7]  Display top 1 match and report its correctness
*
*******************************************************************************/

void TestWindow::on_pushButton_clicked()
{
    //Prompts to select the query image
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


    //Scale and display the query image
    Mat scaledQueryImg;
    cv::resize(queryImg,scaledQueryImg,Size(),0.50,0.33,INTER_LINEAR);
    cv::imshow("Query Image",scaledQueryImg);

    ///////////////// Feature extraction on query image ///////////////////////

    vector<cv::KeyPoint> keypoints;
    Mat matGrayscale;
    Mat descriptor;
    Mat descriptorFM;
    std::vector<std::vector<int> >* pointIdxsOfClusters= new std::vector<std::vector<int> >;
    vector<int> imgHist;

    //create Sift feature point detector and feature extractor
    Ptr<FeatureDetector> detectorFM=xfeatures2d::SiftFeatureDetector::create(0,3,0.08,10,1.6);
    Ptr<DescriptorExtractor> extractorFM=xfeatures2d::SiftDescriptorExtractor::create();

    //create a nearest neighbor matcher
    Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher);
    //Create BoF descriptor extractor
    BOWImgDescriptorExtractor bowDE(extractorFM,matcher);
    //Add vocabulary
    bowDE.setVocabulary(vocabulary);

    //Convert query image to grayscale
    cv::cvtColor(queryImg,matGrayscale,CV_BGR2GRAY);

    //Gaussian Smoothing
    Mat dst;
    cv::blur(matGrayscale,dst,Size(3,3));

    //Binarization
    Mat thresImg;
    cv::threshold(dst,thresImg,127,255,CV_THRESH_BINARY);

    //Detect keypoints and extract descriptors
    detectorFM->detect(thresImg,keypoints);
    extractorFM->compute(thresImg, keypoints,descriptor);

    //Match descriptor to Clusters from vocabulary
    bowDE.compute(descriptor,descriptorFM,pointIdxsOfClusters);

    //Compute histogram for query image
    for (std::vector<std::vector<int> >::iterator it = pointIdxsOfClusters->begin() ; it != pointIdxsOfClusters->end(); ++it){
        std::vector<int> row=*it;
        imgHist.push_back(row.size());
    }

    ///////////////// Compute distance of query image from Histogram database ///////////////////////

    //Loop through each histogram from histDatabase
    //and calculate distance from query image histogram
    vector<double> distVector;
    int cnt=0;
    for (vector<vector<int> >::iterator it1 = histDatabase.begin() ; it1 != histDatabase.end(); ++it1){
        std::vector<int> dbHistVect=*it1;
        double dist=computeDist(imgHist,dbHistVect);
        distVector.push_back(dist);
        cnt++;
    }
    cout<<endl;



    //Sorting according to increasing distance
    //Cosine dist
    int idx[816];
    std::iota(std::begin(idx), std::end(idx), 0);
    std::sort(std::begin(idx), std::end(idx), std::bind(compareX, _1, _2, distVector));

    QStringList pieces = strFileName.split( "/" );
    QString queryFilenm = pieces.value( pieces.length() - 1 );
    queryFilenm.remove(".png");

    cout<<"Query Image: "<<queryFilenm.toStdString()<<endl;

    //////////////// Top5 Output //////////////

    cout << "Top 5 match generated by program...."<<endl;
    cout << "<Matched Form Name>: <Distance>"<<endl;

    QStringList top5;   //Sorted first five forms
    int i;
    for ( i=0; i<5; ){
        QString matchedImage=QString::fromStdString(imgIdDatabase[idx[i]]);

        if(queryFilenm!=matchedImage)
        {
            cout <<matchedImage.toStdString()<<": "<< distVector[idx[i]] << endl;
            top5.append(matchedImage);
            i++;
        }
    }


    //Open forms1.txt to find the actual author of the query Form file
    //forms1.txt File format: <Form_Name> <Writer_Id>
    QFile formFile(form_file_path);
    if(!formFile.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        cout<<"Error: Could not open forms1.txt"<<endl;
        return;
    }

    QTextStream form(&formFile);
    QString writerid;

    //Find writer of the query file
    while(!form.atEnd())
    {
        QString line=form.readLine();
        if(line.split(" ")[0]==queryFilenm)
        {
            writerid=line.split(" ")[1];
            break;
        }
    }

    formFile.close();

    cout<<"True/Actual match from database....."<<endl;
    cout<<"True Writer :"<<writerid.toStdString()<<endl;

    //Open uniq_writer-img_list to find list of documents written by the writer
    //uniq_writer-img_list.txt File format: <Writer_Id>|<list of comma-separated documents>
    QFile uniqWriterImgFile(uniqWriterImgFile_path);
    if(!uniqWriterImgFile.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        cout<<"Error: Could not open uniq_writer_img_list.txt"<<endl;
        return;
    }


    QTextStream uniqWriteImg(&uniqWriterImgFile);
    QStringList actualMatch;

    //Find list of documents written by the writer
    while(!uniqWriteImg.atEnd())
    {
        QString line=uniqWriteImg.readLine();

        if(line.split("|")[0]==writerid)
        {
            //Writer Id match found
            //Get list of docs
            QString tempimglist=line.split("|")[1];
            cout<<"True/Actual match: "<<tempimglist.toStdString()<<endl;

            //Save actualmatch to variable for success rate calculation
            actualMatch=tempimglist.split(",");
            break;
        }
    }

    uniqWriterImgFile.close();

    ////////////////// Success rate calculation //////////////////

    //////////////// Top5 Statistics //////////////

    int flag=0,actual_success=0;
    int actual_match_cnt=actualMatch.size()>5?5:actualMatch.size();

    cout<<"Comparing program output with actual match"<<endl;
    cout<<"<Actual match>: F(Found)/NF(Not found)"<<endl;

    //Compare each actualMatch with top5 match generated by program
    for (int j = 0; j < actualMatch.size(); ++j)
    {
        QString temp=actualMatch[j];

        //Comapring with cosine program output
        if(top5.contains(temp))
        {
            //Program output contains the actual/true match
            flag=1;
            cout<<temp.toStdString()<<": F "<<endl;
            actual_success++;
        }
        else{
            cout<<temp.toStdString()<<": NF "<<endl;
        }
    }

    cout<<"Top 5 Recall: "<<((double)actual_success/actual_match_cnt)*100<<" %"<<endl;
    cout<<"Top 5 Precision: "<<((double)actual_success/5)*100<<" %"<<endl;


    //////////////// Top1 Statistics //////////////

    cout<<endl<<"Top1 Statistics..."<<endl;

    QStringList::iterator it = top5.begin();
    QString top1_temp=*it;
    cout<<"Top 1 match from program :"<<top1_temp.toStdString()<<endl;

    //Display Top1 match
    QString top1_filenm=training_path+"/"+top1_temp+".png";
    Mat top1_match=imread(top1_filenm.toStdString(),0);
    Mat top1_match_scaled;
    cv::resize(top1_match,top1_match_scaled,Size(),0.5,0.33,INTER_LINEAR);
    cv::imshow("Top 1 Match",top1_match_scaled);

    if(actualMatch.contains(top1_temp))
        cout<<"Top 1: correct"<<endl;
    else
        cout<<"Top 1: incorrect"<<endl;

    waitKey(0);
    destroyAllWindows();

    //release memory
    matGrayscale.release();
    dst.release();
    thresImg.release();
    keypoints.clear();
    descriptor.release();
    pointIdxsOfClusters->clear();
    imgHist.clear();
    distVector.clear();

}


/*****************************************************************************
* NAME :            TestWindow::on_pushButton_4_clicked
*
* DESCRIPTION :     This function finds the documents written by the author
*                   of all test images.
*
* INPUTS :
*       PARAMETERS:
*           None
*       GLOBALS :
*           None
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  Create BoF descriptor extractor and add vocabulary
*                   [2]  Loop through all test image
*                   [3]     Load image
*                   [4]     Feature extraction on query image
*                   [4]     Calculate cosine and chi-square distance from
*                           histogram database of training images
*                   [5]     Determine both cosine and chi-square top 5 match
*                   [6]  Calculate success statistics for top 5 & top3 for
*                        both distance metric.
*
*******************************************************************************/

void TestWindow::on_pushButton_4_clicked()
{

    //create Sift feature point detector and feature extractor
    Ptr<FeatureDetector> detectorFM=xfeatures2d::SiftFeatureDetector::create(0,3,0.08,10,1.6);
    Ptr<DescriptorExtractor> extractorFM=xfeatures2d::SiftDescriptorExtractor::create();

    //Create a nearest neighbor matcher
    Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher);
    //Create BoF descriptor extractor
    BOWImgDescriptorExtractor bowDE(extractorFM,matcher);

    bowDE.setVocabulary(vocabulary);


    QDir dir(test_path);

    if (!dir.exists())
        qWarning("Cannot find the example directory");
    else
    {
        cout<<"Test dir opened"<<endl;

        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        int progress=0;
        //Declaring sorted distance vector index array
        int idx[816],idxChi[816];
        QVector<QStringList> top5_match;
        QVector<QStringList> top5_match_Chi;


        Mat img;
        Mat matGrayscale;
        Mat descriptor;
        Mat descriptorFM;
        vector<cv::KeyPoint> keypoints;
        std::vector<std::vector<int> >* pointIdxsOfClusters= new std::vector<std::vector<int> >;
        vector<int> imgHist;

        //Loop through all test images
        foreach(QString dirFile, dir.entryList())
        {

            cout<<"Progress: "<<progress<<endl;
            QString filenameWithPath=dir.absolutePath()+"/"+dirFile;

            //Load image
            img=cv::imread(filenameWithPath.toStdString());
            cout<<"Test Image: "<<dirFile.toStdString()<<" | ";

            cv::cvtColor(img,matGrayscale,CV_BGR2GRAY);

            //Smoothing
            Mat dst;
            cv::blur(matGrayscale,dst,Size(3,3));

            //Binarization
            Mat thresImg;
            cv::threshold(dst,thresImg,127,255,CV_THRESH_BINARY);

            //Detect keypoints, descriptors and Map to cluster centers
            detectorFM->detect(thresImg,keypoints);
            extractorFM->compute(thresImg, keypoints,descriptor);

            //Match descriptor to Clusters from vocabulary
            bowDE.compute(descriptor,descriptorFM,pointIdxsOfClusters);

            //compute histogram for query image
            std::vector<int> row;
            for (std::vector<std::vector<int> >::iterator it2 = pointIdxsOfClusters->begin() ; it2 != pointIdxsOfClusters->end(); ++it2){
                row=*it2;
                imgHist.push_back(row.size());
            }

            //Loop through each histogram from histDatabase
            //and calculate cosine distance and ch-square distance from query image histogram
            int cnt=0;
            vector<double> distVector,distVectorChi;
            std::vector<int> dbHistVect;
            double dist,distChi;

            for (vector<vector<int> >::iterator it3 = histDatabase.begin() ; it3 != histDatabase.end(); ++it3){
                dbHistVect=*it3;
                //cosine distance
                dist=computeDist(imgHist,dbHistVect);
                //chi square distance
                distChi=computeDistChi(imgHist,dbHistVect);
                distVector.push_back(dist);
                distVectorChi.push_back(distChi);
                cnt++;
                dbHistVect.clear();
            }

            //Sorting according to increasing distance
            //Cosine dist
            std::iota(std::begin(idx), std::end(idx), 0);
            std::sort(std::begin(idx), std::end(idx), std::bind(compareX, _1, _2, distVector));
            //Chi square dist
            std::iota(std::begin(idxChi), std::end(idxChi), 0);
            std::sort(std::begin(idxChi), std::end(idxChi), std::bind(compareX, _1, _2, distVectorChi));

            cout << "Top five match (Cos Sim) - ";
            QStringList top5,top5Chi;

            for (int i=0; i<5; i++){
                cout <<imgIdDatabase[idx[i]]<<":"<< distVector[idx[i]] << ",";

                //top5 based on cosine similarity distance metric
                QString matchedImg=QString::fromStdString(imgIdDatabase[idx[i]]);
                top5.append(matchedImg);

                //top5 based on chi square distance metric
                matchedImg=QString::fromStdString(imgIdDatabase[idxChi[i]]);
                top5Chi.append(matchedImg);

            }

            cout<<endl;

            //add to list of matched images for all test images
            top5_match.push_back(top5);
            top5_match_Chi.push_back(top5Chi);

            //release memory
            img.release();
            matGrayscale.release();
            dst.release();
            thresImg.release();
            keypoints.clear();
            descriptor.release();
            descriptorFM.release();
            pointIdxsOfClusters->clear();
            imgHist.clear();
            distVector.clear();

            progress++;
        }


        cout<<"All files tested"<<endl;

        ///////////////// Calculating Test Statistics for all test images ///////////////////////////

        cout<<endl<<"Calculating test statistics..."<<endl;
        int i=0, success_top5=0,successChi_top5=0,success_top3=0,successChi_top3=0;
        double recall, precision;
        double success_rate_top5,successChi_rate_top5,success_rate_top3,successChi_rate_top3;
        int sum_actual_success_top5=0, sum_actual_success_chi_top5=0,sum_actual_success_top3=0, sum_actual_success_chi_top3=0;
        int sum_actual_match_top5_cnt=0,sum_actual_match_top3_cnt=0;

        QString top5filenm="/top5_"+q_clust_center+"_CT_cosinesim.txt";
        QString top5Chifilenm="/top5"+q_clust_center+"CT_chisquare.txt";
        cout<<"Top 5 filename: "<<top5filenm.toStdString()<<" "<<top5Chifilenm.toStdString()<<endl;

        QFile top5file(out_dir+top5filenm);
        QFile top5Chifile(out_dir+top5Chifilenm);

        //Open files to write test statistics
        if (!top5file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            cout<<"Could not open top5 file"<<endl;
            return;
        }
        if (!top5Chifile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            cout<<"Could not open top 5 Chi file"<<endl;
            return;
        }

        QTextStream outTop5(&top5file);
        QTextStream outTop5Chi(&top5Chifile);
        outTop5<<"filename|writerid|Program Match|Actual Match|Test Statistics<True Match form name: F/NF(Found/Not Found)>|Successful Match|Actual Match|"<<endl;
        outTop5Chi<<"filename|writerid|Program Match|Actual Match"<<endl;

        //Loop through all test images
        foreach(QString dirFile, dir.entryList())
        {
            QString filename=dirFile.remove(".png");
            QStringList top5=top5_match[i];
            QStringList top5Chi=top5_match_Chi[i];
            QStringList top3,top3Chi;

            for (QStringList::iterator it = top5.begin();it < top5.begin()+3; ++it)
                top3.append(*it);
            for (QStringList::iterator it = top5Chi.begin();it < top5Chi.begin()+3; ++it)
                top3Chi.append(*it);


            //Open forms1.txt to find the actual author of the Form file
            //File format: <Form_Name> <Writer_Id>
            QFile formFile(form_file_path);
            if(!formFile.open(QIODevice::ReadOnly| QIODevice::Text))
            {
                cout<<"Could not open forms1.txt"<<endl;
                return;
            }

            QTextStream form(&formFile);
            QString writerid;

            //Find writer of the query document
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

            cout<<endl<<"Img: "<<filename.toStdString()<<"|Writer Id: "<<writerid.toStdString()<<"|Program Match: "<<(top5.join(",")).toStdString()<<"|Actual Match: ";
            outTop5<<filename<<"|"<<writerid<<"|"<<top5.join(",")<<"|";
            outTop5Chi<<filename<<"|"<<writerid<<"|"<<top5Chi.join(",")<<"|";

            //Open uniq_writer-img_list to find list of documents written by the writer
            //uniq_writer-img_list.txt File format: <Writer_Id>|<list of comma-separated documents>
            QFile uniqWriterImgFile(uniqWriterImgFile_path);
            if(!uniqWriterImgFile.open(QIODevice::ReadOnly| QIODevice::Text))
            {
                cout<<"Could not open uniq_writer_img_list.txt"<<endl;
                return;
            }

            QTextStream uniqWriteImg(&uniqWriterImgFile);
            QStringList actualMatch;

            //Find list of documents written by the writer
            while(!uniqWriteImg.atEnd())
            {
                QString line=uniqWriteImg.readLine();

                if(line.split("|")[0]==writerid)
                {
                    //Writer Id match found
                    //Get list of docs
                    QString tempimglist=line.split("|")[1];
                    cout<<tempimglist.toStdString();

                    //Write actual match of the query image to file
                    outTop5<<tempimglist<<"|";
                    outTop5Chi<<tempimglist<<"|";
                    //save actualmatch to variable for success rate calculation
                    actualMatch=tempimglist.split(",");
                    break;
                }
            }

            uniqWriterImgFile.close();

            /////////////////// Calculate success rate ///////////////////

            int flagTop5=0,flagChiTop5=0,flagTop3=0,flagChiTop3=0;
            int actual_success_top5=0, actual_success_chi_top5=0;
            int actual_success_top3=0, actual_success_chi_top3=0, actual_match_top5_cnt,actual_match_top3_cnt;
            cout<<"|Test Stat: ";

            actual_match_top5_cnt=actualMatch.size()>5?5:actualMatch.size();
            actual_match_top3_cnt=actualMatch.size()>3?3:actualMatch.size();

            //Compare each actualMatch with top5 (CosineSim & Chi square)
            for (int j = 0; j < actualMatch.size(); ++j)
            {
                QString temp=actualMatch[j];

                //////////////// Top5 Statistics //////////////
                //Comapring with cosine program output
                if(top5.contains(temp))
                {
                    //Program output contains the actual/true match
                    flagTop5=1;
                    cout<<temp.toStdString()<<": F ,";
                    outTop5<<temp<<": F ,";
                    actual_success_top5++;
                }
                else{
                    //Program output does not contain the actual/true match
                    outTop5<<temp<<": NF ,";
                }

                //Comapring with chi sqaure program output
                if(top5Chi.contains(temp))
                {
                    flagChiTop5=1;
                    outTop5Chi<<temp<<": F ,";
                    actual_success_chi_top5++;
                }
                else
                {
                    //Program output does not contain the actual/true match
                    outTop5Chi<<temp<<": NF ,";
                }


                //////////////// Top3 Statistics //////////////
                //Comapring with cosine program output
                if(top3.contains(temp))
                {
                    //Program output contains the actual/true match
                    flagTop3=1;
                    actual_success_top3++;
                }

                //Comparing with chi sqaure program output
                if(top3Chi.contains(temp))
                {
                    flagChiTop3=1;
                    actual_success_chi_top3++;
                }

            }

            outTop5<<"|"<<actual_success_top5<<"|"<<actual_match_top5_cnt<<endl;
            outTop5Chi<<"|"<<actual_success_chi_top5<<"|"<<actual_match_top3_cnt<<endl;

            //Updating sum variables for calculating recall and precision
            sum_actual_success_top5+=actual_success_top5;
            sum_actual_success_chi_top5+=actual_success_chi_top5;
            sum_actual_success_top3+=actual_success_top3;
            sum_actual_success_chi_top3+=actual_success_chi_top3;
            sum_actual_match_top5_cnt+=actual_match_top5_cnt;
            sum_actual_match_top3_cnt+=actual_match_top3_cnt;


            //Overall success incremented if at least one match is found
            //Top 5
            if(flagTop5==1)
                success_top5++;
            else
                outTop5<<"|No Match";

            if(flagChiTop5==1)
                successChi_top5++;
            else
                outTop5Chi<<"|No Match";

            //Top 3
            if(flagTop3==1)
                success_top3++;
            if(flagChiTop3==1)
                successChi_top3++;

            i++;

            //release memory
            top5.clear();
            top5Chi.clear();
            top3.clear();
            top3Chi.clear();

            cout<<endl;

        }

        success_rate_top5=((double)success_top5/progress)*100;
        successChi_rate_top5=((double)successChi_top5/progress)*100;

        success_rate_top3=((double)success_top3/progress)*100;
        successChi_rate_top3=((double)successChi_top3/progress)*100;

        cout<<endl<<"Top 5 Statistics: "<<endl;
        cout<<"======================="<<endl;
        cout<<"1. Cosine Similarity metric:"<<endl;
        cout<<"Success Rate: "<<success_rate_top5<<"%"<<endl;

        recall=((double)sum_actual_success_top5/sum_actual_match_top5_cnt)*100;
        precision=((double)sum_actual_success_top5/(5*progress))*100;

        cout<<"Recall: "<<recall<<"%"<<endl;
        cout<<"Precision:"<<precision<<"%"<<endl;

        outTop5<<endl<<"Top 5 Statistics: "<<endl;
        outTop5<<"Cosine Similarity metric"<<endl;
        outTop5<<"Success Rate: "<<success_rate_top5<<"%"<<endl;
        outTop5<<"Recall: "<<recall<<"%"<<endl;
        outTop5<<"Precision: "<<precision<<"%"<<endl;

        cout<<endl<<"2. Chi Square distance metirc:"<<endl;
        cout<<"Success Rate: "<<successChi_rate_top5<<"%"<<endl;

        recall=((double)sum_actual_success_chi_top5/sum_actual_match_top5_cnt)*100;
        precision=((double)sum_actual_success_chi_top5/(5*progress))*100;

        cout<<"Recall: "<<recall<<"%"<<endl;
        cout<<"Precision:"<<precision<<"%"<<endl;

        outTop5Chi<<endl<<"Top 5 Statistics: "<<endl;
        outTop5Chi<<"Chi Square distance metric"<<endl;
        outTop5Chi<<"Success Rate : "<<successChi_rate_top5<<"%"<<endl;
        outTop5Chi<<"Recall: "<<recall<<"%"<<endl;
        outTop5Chi<<"Precision:"<<precision<<"%"<<endl;

        cout<<endl<<"Top 3 Statistics: "<<endl;
        cout<<"======================="<<endl;
        cout<<"1. Cosine Similarity metric:"<<endl;
        cout<<"Success Rate: "<<success_rate_top3<<"%"<<endl;

        recall=((double)sum_actual_success_top3/sum_actual_match_top3_cnt)*100;
        precision=((double)sum_actual_success_top3/(3*progress))*100;

        cout<<"Recall: "<<recall<<"%"<<endl;
        cout<<"Precision:"<<precision<<"%"<<endl;

        outTop5<<"Top 3 Statistics: "<<endl;
        outTop5<<"1. Cosine Similarity metric:"<<endl;
        outTop5<<"Success Rate: "<<success_rate_top3<<"%"<<endl;
        outTop5<<"Recall: "<<recall<<"%"<<endl;
        outTop5<<"Precision:"<<precision<<"%"<<endl;

        cout<<endl<<"2. Chi Square distance metirc:"<<endl;
        cout<<"Success Rate: "<<successChi_rate_top3<<"%"<<endl;

        recall=((double)sum_actual_success_chi_top3/sum_actual_match_top3_cnt)*100;
        precision=((double)sum_actual_success_chi_top3/(3*progress))*100;

        cout<<"Recall: "<<recall<<"%"<<endl;
        cout<<"Precision:"<<precision<<"%"<<endl;

        outTop5Chi<<"Top 3 Statistics: "<<endl;
        outTop5Chi<<"2. Chi Square distance metric:"<<endl;
        outTop5Chi<<"Success Rate: "<<successChi_rate_top3<<"%"<<endl;
        outTop5Chi<<"Recall: "<<recall<<"%"<<endl;
        outTop5Chi<<"Precision:"<<precision<<"%"<<endl;

        top5file.close();
        top5Chifile.close();
    }

}


/*****************************************************************************
* NAME :            TestWindow::computeDistChi
*
* DESCRIPTION :     This function calculates the chi-square distance between two
*                   histogram vectors.
*
* INPUTS :
*       PARAMETERS:
*           vector<int>     queryImgHist        histogram of query image
*           vector<int>     dbImgHist           histogram of training image
*       GLOBALS :
*           None
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  Calculates chi-square distance between two vectors
*
*******************************************************************************/

double TestWindow::computeDistChi(vector<int> queryImgHist,vector<int> dbImgHist){

    double q,d,sum=0;

    for (int i=0; i<queryImgHist.size();i++){
        q=queryImgHist[i];
        d=dbImgHist[i];
        if((d+q)!=0){
            sum+=pow((d-q),2)/(d+q);
        }
    }
    return sqrt(sum);
}


/*****************************************************************************
* NAME :            TestWindow::computeDist
*
* DESCRIPTION :     This function calculates the cosine distance between two
*                   histogram vectors.
*
* INPUTS :
*       PARAMETERS:
*           vector<int>     queryImgHist        histogram of query image
*           vector<int>     dbImgHist           histogram of training image
*       GLOBALS :
*           None
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  Calculates cosine distance between two vectors based on
*                        the following formula
*                        cos(θ)=(A.B)/(||A|| ||B||)
*
*******************************************************************************/
double TestWindow::computeDist(vector<int> queryImgHist,vector<int> dbImgHist){

    double dot=0, denom_a=0, denom_b=0;

    for(int i=0; i< queryImgHist.size();i++)
    {
        dot+=queryImgHist[i]*dbImgHist[i];
        denom_a+=queryImgHist[i]*queryImgHist[i];
        denom_b+=dbImgHist[i]*dbImgHist[i];
    }
    denom_a=sqrt(denom_a);
    denom_b=sqrt(denom_b);

    return acos(dot/(denom_a*denom_b));

}

/*
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
*/
