/***********************************************************************
* FILENAME :        mainwindow.cpp
*
* DESCRIPTION :
*       This window is for training phase. It contains a simple UI which
*       serves two tasks - creation of Vocabulary and Histogram database.
*
* SLOTS :
*       void    on_pushButton_clicked( )
*       void    on_SelectVocabulary_clicked()
*       void    on_CreateHistDB_clicked()
*
* FUNCTIONS:
*       Mat     cropImage(Mat inputImage)
*       void    preprocessIAM()
*       vector<vector<float> > kmeansClusterMultiDim(int k)
*       double  computeDist(vector<float> vec1,vector<float> vec2)
*       vector<float> calcAvgVec(vector<vector<float> > obj)
*
************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QFileDialog>
#include<QtCore>
#include<QDir>
#include<opencv2/highgui.hpp>
#include<opencv2/core.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/features2d/features2d.hpp>
#include<opencv2/xfeatures2d.hpp>
#include<iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include "testwindow.h"
#include <cstdio>
#include <ctime>
#include<random>
#include<omp.h>

using namespace std;
using namespace cv;


RNG rng(12345);
extern QString training_path;
extern QString test_path;
extern QString out_dir;
extern QString single_sample_list;
extern bool featureFlag;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    TestWindow *objTest=new TestWindow();
    objTest->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/***************************************************************************
* NAME :            MainWindow::cropImage
*
* DESCRIPTION :     Crops Image after detecting the main text area.
*                   This function is specific to IAM dataset as it is invoked
*                   from preprocessIAM(). IAM forms contain two sections
*                   separated by horizontal lines. The section between first
*                   two horizontal lines is typewritten and next section
*                   between second and third lines is handwritten. This function
*                   detects the handwritten area and crops it.
*
* INPUTS :
*       PARAMETERS:
*           Mat         inputImage
*       GLOBALS :
*           None
* OUTPUTS :
*       RETURN :
*           Type:       Mat         Cropped Image
* PROCESS :
*                   [1]  Canny Operation on input image
*                   [2]  Detect lines using Hough transform
*                   [3]  If no lines are detected, crop at defailt location
*                   [4]  Else identify horizontal lines
*                   [5]     For range of threshold and sigma values
*                   [6]         Detect SIFT keypoints
*                   [7]         Draw Keypoints on Image and Save to disk
*                   [8]         Display Image
*
****************************************************************************/


cv::Mat MainWindow::cropImage(cv::Mat inputImage){
    cv::Mat cannyOutput;
    vector<Vec2f> lines;
    vector<int> rows;
    cv::Mat croppedImg,temp;

    //Canny operation on input image
    cv::Canny(inputImage,cannyOutput,127,255,3);

    //Detect lines using Hough Transform with threshold=700
    cv::HoughLines(cannyOutput,lines,1, CV_PI/180, 700, 0, 0);


    if(lines.size()==0)
    {
        //If no lines are detected,
        //crop at default locations
        cout<<"cropping at default location"<<endl;
        temp=inputImage(Rect(0,280,inputImage.cols,2180));
        temp.copyTo(croppedImg);
    }
    else{

        //Filter horizontal lines
        for( size_t i = 0; i < lines.size(); i++ )
          {
             float rho = lines[i][0], theta = lines[i][1];

             if ( theta>1.56 and theta<1.58)
             {
                 double b = sin(theta);
                 //y0: row corresponding to the line or distance of the line from top
                 double y0 = b*rho;
                 rows.push_back(y0);
             }
          }


        if(rows.size()>0)
        {

            //Create histogram of the row numbers corresponding to the horizontal lines

            double bucket_size=50;
            int number_of_buckets=(int)(inputImage.rows/bucket_size);
            vector<int> histogram(number_of_buckets);

            for(vector<int>::iterator it=rows.begin();it != rows.end(); ++it)
            {
                int row=*it;
                int bucket=(int)(row/bucket_size);
                histogram[bucket]+=1;
            }

            int nonzero_cnt=histogram.size()-count(histogram.begin(),histogram.end(),0);

            //As there should be two major horizontal lines in the main text area,
            //If Non zero count of the histogram size >=2 return last 2 lines
            if(nonzero_cnt>=2)
            {
                int flag=0;
                int hlines[2];
                //iterating histogram in reverse and return first two lines
                for(vector<int>::reverse_iterator rit=histogram.rbegin();rit!=histogram.rend();++rit)
                {
                    if(flag>=2)
                        break;

                    if(*rit>0)
                    {
                        flag++;
                        hlines[2-flag]=bucket_size*(histogram.size() -1 - (rit -histogram.rbegin()));
                    }
                }

                //crop image area between the two detected lines
                temp=inputImage(Rect(0,hlines[0],inputImage.cols,(hlines[1]-hlines[0])));
                temp.copyTo(croppedImg);
            }
            else{
                //If Non zero count of the histogram is less than 2
                //crop at default locations
                cout<<"cropping at default location"<<endl;
                temp=inputImage(Rect(0,280,inputImage.cols,2180));
                temp.copyTo(croppedImg);
            }
        }


    }

    return croppedImg;

    // OLD LOGIC - DISCARDED
    //Get contour with maximum area

/*
    for( int i = 0; i< contours.size(); i++ )
    {
        //  Find the area of contour
        double area=contourArea(contours[i],false);

        if(area>largest_area){
            largest_area=area;
            cout<<i<<" area  "<<area<<endl;
            // Store the index of largest contour
            largest_contour_index=i;
            // Find the bounding rectangle for biggest contour
            bounding_rect=boundingRect(contours[i]);
        }
    }

    //cv::drawContours(inputImage, contours, largest_contour_index, (255,0,0),5);
    cout<<"Contour Size:"<<contours.size()<<endl;
    cout<<"Bounding Rectangle :"<<bounding_rect.x<<" "<<bounding_rect.y<<" "<<bounding_rect.width<<" "<<bounding_rect.height<<endl;

    cv::imwrite("CroppedManuscript.jpg",cv::Mat(inputImage,bounding_rect));
*/
}

/*******************************************************************************
* NAME :            MainWindow::preprocessIAM
*
* DESCRIPTION :     This function is specific to IAM dataset.IAM forms contain two sections
*                   separated by horizontal lines. The section between first
*                   two horizontal lines is typewritten and next section
*                   between second and third lines is handwritten. This function
*                   preprocess files and detects main text area.
*
* INPUTS :
*       PARAMETERS:
*           None
*       GLOBALS :
*           QString     training_path
*           QString     test_path
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  For all training images
*                   [2]     Load Image
*                   [3]     Remove shadow on left and fixed heading
*                   [4]     Check if file is a single sample file
*                   [5]         If yes, crop and split image for training and test
*                   [6]         Else, crop image
*
**********************************************************************************/

void MainWindow::preprocessIAM(){

    QDir dir(training_path);

    if (!dir.exists())
        qWarning("Cannot find the training directory");
    else
    {
        cout<<"Training directory opened"<<endl;

        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        //Loop through all files in directory
        foreach(QString dirFile, dir.entryList())
        {
             cout<<"File name: "<<dirFile.toStdString()<<endl;
             QString filenameWithPath=dir.absolutePath()+"/"+dirFile;

             //Load Image
             Mat img=cv::imread(filenameWithPath.toStdString());

             //Removing shadow on left and fixed heading - removes top most horizontal line
             Mat tmpImg=img(Rect(100,370,(img.cols-100),(img.rows-370)));

             //Check if the image file is in single sample list
             //single sample list: list of files which are the only samples written by authors
             QFile checkListFile(single_sample_list);

             if (!checkListFile.open(QIODevice::ReadOnly | QIODevice::Text))
             {
                 cout<<"Could not open single_sample_list.txt"<<endl;
                 return;
             }

             QTextStream in(&checkListFile);
             bool flag=false;

             while (!in.atEnd()) {
                 QString line = in.readLine();
                 if(line==dirFile)
                 {
                     flag=true;
                     break;
                 }
             }
             checkListFile.close();

             Mat croppedImage;
             if(flag==false)
             {
                 //Crop Image
                 croppedImage=cropImage(tmpImg);
                 //save cropped image to disk
                 cv::imwrite(filenameWithPath.toStdString(),croppedImage);
             }
             else
             {
                 //Single sample file processing
                 cout<<"single sample file: "<<dirFile.toStdString()<<endl;

                 //Crop image and split into two parts - one for training and one for testing
                 croppedImage=cropImage(tmpImg);
                 Mat part1_trainimg=croppedImage(Rect(0,0,croppedImage.cols,(croppedImage.rows/2)));
                 Mat part2_testimg=croppedImage(Rect(0,(croppedImage.rows/2)+1,croppedImage.cols,(croppedImage.rows/2)-1));

                 //save cropped image to disk
                 //training part
                 cv::imwrite(filenameWithPath.toStdString(),part1_trainimg);
                 //test part
                 string tstFileName=test_path.toStdString()+dirFile.toStdString();
                 cv::imwrite(tstFileName,part2_testimg);
             }
        }
        cout<<"All files processed"<<endl;
     }

}

/*******************************************************************************
* NAME :            MainWindow::computeDist
*
* DESCRIPTION :     This function computes the cosine distance between two input vectors
*
* INPUTS :
*       PARAMETERS:
*           vector<float>   vec1
*           vector<float>   vec2
*       GLOBALS :
*           None
* OUTPUTS :
*       RETURN :
*           Type:   double      Angular distance between the input vectors
* PROCESS :
*                   [1]  Calculate cosine distance between the vectors using
*                        the following formula:
*                        cos(θ)=(A.B)/(||A|| ||B||)
*
**********************************************************************************/
double MainWindow::computeDist(vector<float> vec1,vector<float>vec2){
    double dot=0, denom_a=0, denom_b=0;

    for(int i=0; i< vec1.size();i++)
    {
        dot+=vec1[i]*vec2[i];
        denom_a+=vec1[i]*vec1[i];
        denom_b+=vec2[i]*vec2[i];
    }
    denom_a=sqrt(denom_a);
    denom_b=sqrt(denom_b);

    return abs(acos(dot/(denom_a*denom_b)));

}

/*******************************************************************************
* NAME :            MainWindow::on_pushButton_clicked
*
* DESCRIPTION :     This function computes the vocabulary for training images
*
* INPUTS :
*       PARAMETERS:
*           None
*       GLOBALS :
*           out_dir         Path of out directory
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  Invoke preprocessIAM() (Executed only in the first execution)
*                   [2]  During first execution
*                   [3]     Prompt to choose training images
*                   [3]     For all selected images
*                   [4]         Load Image in grayscale
*                   [5]         Gaussian Smoothing & Binarization
*                   [6]         Feature Extraction - Keypoint detection & descriptor
*                   [7]         Add descriptor to the master list of features
*                   [8]     Save master list of features to disk for future use
*                   [9]  From second execution
*                   [10]    Load saved master list of features
*                   [11] For a range of cluster centers
*                   [12]    Run clustering algorithm on features
*                   [13]    Save vocabulary to disk in out_dir
***************************************************************************************/

void MainWindow::on_pushButton_clicked()
{
    //////////////////////////// Preprocess Training Images - One Time /////////////////////////////
    //preprocessIAM();
    cout<<"Preprocessing completed"<<endl;

    //////////////////////////// File Dialog and Cleaning //////////////////////////////////////////

    if(featureFlag==true)
    {
        //During first execution
        //Prompt to select training images
        QStringList files = QFileDialog::getOpenFileNames(
                    this,
                    "Select one or more files to open",
                    ".",
                    "Images (*.png *.xpm *.jpg)");
        QStringList list = files;

        if(files.size()== 0){
            ui->lblChosenDir->setText("Training Images not chosen");
            return;
        }

        cout<<"Number of files chosen - "<<list.count()<<endl;

        QString fileErrorMsg="Error: List of images not loaded - ";
        int cnt=0;
        int totalKeypt=0;
        bool flag=false;
        clock_t start = std::clock();

        //Loop through all selected images
#pragma omp parallel for
        {
            for (QStringList::Iterator it = list.begin();it != list.end(); ++it) {
                cv::Mat matOriginal;
                cv::Mat matGrayscale;
                vector<cv::KeyPoint> keypoints;
                Mat descriptor;

                QString filename=*it;

                //Load Image
                matOriginal=cv::imread(filename.toStdString());

                if(matOriginal.empty()){
                    flag=true;
                    fileErrorMsg.append(" Image "+QFileInfo(filename).fileName());
                }
                else{

                    cv::cvtColor(matOriginal,matGrayscale,CV_BGR2GRAY);

                    //Gaussian Smoothing
                    Mat dst;
                    cv::blur(matGrayscale,dst,Size(3,3));

                    //Binarization
                    Mat thresImg;
                    cv::threshold(dst,thresImg,127,255,CV_THRESH_BINARY);

                    //Detect SIFT Key Points with parameters decided from 'keypointdetection'
                    Ptr<cv::xfeatures2d::SiftFeatureDetector> detector=xfeatures2d::SiftFeatureDetector::create(0,3,0.08,10,1.6);
                    //Ptr<cv::xfeatures2d::SiftFeatureDetector> detector=xfeatures2d::SiftFeatureDetector::create();
                    detector->detect(thresImg,keypoints);

                    cout<<"Number of keypoints detected: "<<keypoints.size()<<endl;
                    //cout<<"Keypoint 1 coordinates:"<<keypoints[1].pt<<" Keypoint 1 angle:"<<keypoints[1].angle<<" Keypoint 1 scale/octave:"<<keypoints[1].octave<<endl;

                    //Calculating average keypoints per image
                    totalKeypt+=keypoints.size();
                    cnt++;

                    //Extracting descriptors
                    Ptr<cv::xfeatures2d::SiftDescriptorExtractor> extractor=xfeatures2d::SiftDescriptorExtractor::create();
                    extractor->compute(thresImg, keypoints,descriptor);
                    //cout<<"Descriptor size:"<<descriptor.rows<<" x "<<descriptor.cols<<endl<<"Descriptor: "<<descriptor.row(1)<<endl;

                    //Add descriptor to the master list of features
                    featuresUnclustered.push_back(descriptor);
                    cout<<"Image: "<<cnt<<endl;

                    //release memory
                    detector.release();
                    extractor.release();
                }

                //release memory
                matOriginal.release();
                matGrayscale.release();
                descriptor.release();
                keypoints.clear();
                keypoints.shrink_to_fit();
            }
        }



        double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
        cout<<"Time for feature extraction: "<< duration <<endl;

        //Report error if any file could not be loaded and processed
        if(flag==true)
            cout<<"Reporting Errors: "<<fileErrorMsg.toStdString()<<endl;

        cout<<"Average number of keypoints per image: "<<(double)totalKeypt/cnt<<endl;
        cout<<"Total number of unclustered Descriptors : "<<featuresUnclustered.size()<<endl<<endl;

        //Backup master list of features for future use
        QString featuresFileName=out_dir+"/"+"FeatureUnclustered.yml";

        FileStorage fsfu(featuresFileName.toStdString(), FileStorage::WRITE);
        try{
            fsfu << "feature" << featuresUnclustered;
        }
        catch(Exception e)
        {
            cout<<"Error : Could not write the features to disk - "<<e.msg<<endl;
            exit(1);
        }

        fsfu.release();
    }
    else{

        //From second execution

        //Prompt to select extracted master feature file from disk
        cout<<"Selecting Feature unclustered file"<<endl;
        QString strFeatureFileName= QFileDialog::getOpenFileName();
        cout<<strFeatureFileName.toStdString()<<endl;

        if(strFeatureFileName== ""){
            cout<<"File not chosen"<<endl;
            ui->lblChosenDir->setText("Feature file not chosen");
            return;
        }

        //Load master features
        FileStorage fs;
        fs.open(strFeatureFileName.toStdString(), FileStorage::READ);

        if (!fs.isOpened())
        {
            cout << "Failed to open " << strFeatureFileName.toStdString() << endl;
            return;
        }
        cout<<"File opened"<<endl;

        fs["feature"]>>featuresUnclustered;

        if(featuresUnclustered.empty()){
            cout<<"Error: Could not load feature"<<endl;
            return;
        }
        cout<<"Feature unclustered loaded successfully - "<<featuresUnclustered.rows<<","<<featuresUnclustered.cols<<endl;
        fs.release();

    }


    //////////////////////////// Clustering and Vocabulary Creation //////////////////////////////////////////

    int clusterCenter[1]={600};       //{200,300,400,600,800,1000,1500};
    cv::TermCriteria tc(CV_TERMCRIT_ITER,100,0.001);
    int retries=1;
    int flags=cv::KMEANS_PP_CENTERS;

    //Create vocabulary for range of clusters
    for(int i=0;i<1;i++){

        cout<<"Cluster Center: "<<clusterCenter[i]<<endl;

        BOWKMeansTrainer bowTrainer(clusterCenter[i],tc,retries,flags);
        bowTrainer.add(featuresUnclustered);

        clock_t start=std::clock();

        //Cluster the feature vectors using Kmeans++(flags=cv::KMEANS_PP_CENTERS)
        #pragma omp parallel
        Mat vocabulary=bowTrainer.cluster();

        double duration = (( std::clock() - start ) / (double) CLOCKS_PER_SEC) / 60;
        cout<<"Time for clustering in minutes: "<< duration <<endl;

        cout<<"Vocabulary generated for "<<clusterCenter[i]<<"cluster centers"<<endl;
        cout<<"Vocabulary size:"<<vocabulary.size()<<endl;

        //Save vocabulary to disk
        QString vocabularyFileName=out_dir+"/"+"dictionary_"+QString::number(clusterCenter[i])+".yml";
        cout<<"Vocabulary file name: "<<vocabularyFileName.toStdString()<<endl;

        FileStorage fs(vocabularyFileName.toStdString(), FileStorage::WRITE);
        try{
            fs << "vocabulary" << vocabulary;
        }
        catch(Exception e)
        {
            cout<<"Error : Could not write the vocabulary to disk - "<<e.msg<<endl;
            exit(1);
        }

        fs.release();
        cout<<"Vocabulary written to disk"<<endl;

        //Release memory
        bowTrainer.clear();
        vocabulary.release();

    }
    featuresUnclustered.release();

    cout<<"Proceed with Histogram Database Creation "<<endl;

    //////////////////////////// OWN KMEANS IMPLEMENTATION - DISCARDED //////////////////////
    //vector<vector<float> > vocabulary=kmeansClusterMultiDim(clusterCenter);

    /*Mat dictionary(vocabulary.size(), vocabulary.at(0).size(), CV_32F);

    for(int i=0; i<dictionary.rows; ++i)
         for(int j=0; j<dictionary.cols; ++j)
              dictionary.at<float>(i, j) = vocabulary.at(i).at(j);

    cout<<"Converted to Mat"<<endl;*/

    ////////////////////////////////////////////////////////////////////////////////////////

}


/*******************************************************************************
* NAME :            MainWindow::on_SelectVocabulary_clicked
*
* DESCRIPTION :     This function selects the vocabulary files from disk
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
*                   [1]  Selects the vocabulary files from disk
*                   [2]  Saves the file path in class member variable strVocabularyFilelist
*
***************************************************************************************/

void MainWindow::on_SelectVocabulary_clicked()
{
    //Prompt to select files
    QStringList files = QFileDialog::getOpenFileNames(
                this,
                "Select one or more vocabulary files to open",
                ".");

    //Saves list of files chosen
    strVocabularyFilelist = files;

    if(files.size()== 0){
        ui->lblVocabulary->setText("Training Images not chosen");
        return;
    }

    //enable CreateHistDB
    ui->CreateHistDB->setEnabled(true);

}


/*******************************************************************************
* NAME :            MainWindow::on_CreateHistDB_clicked
*
* DESCRIPTION :     This function creates histogram of the training images,
*                   based on the chosen vocabulary files.
*
* INPUTS :
*       PARAMETERS:
*           None
*       GLOBALS :
*           training_path   Path of training images
*           out_dir         Path of out directory
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  For each vocabulary file representing different codebook size
*                   [2]     Load vocabular file
*                   [3]     Create BoF descriptor extractor
*                   [3]     Add vocabulary to BoF descriptor extractor
*                   [4]     Loop through all training images
*                   [5]         Gaussian Smoothing & Binarization
*                   [6]         Feature Extraction - Keypoint detection & descriptor
*                   [7]         Match descriptor to Clusters from vocabulary
*                   [8]         Compute histogram of the mapping to clusters
*                   [9]         Save Image identifer and corresponding histogram in master lists.
*                   [10]    Save histograms generated for all images to disk
***************************************************************************************/

void MainWindow::on_CreateHistDB_clicked()
{
    //For each vocabulary files
    for (QStringList::Iterator it = strVocabularyFilelist.begin();it != strVocabularyFilelist.end(); ++it) {

        QString vocabularFileName=*it;
        cout<<"Vocabular file: "<<vocabularFileName.toStdString()<<endl;

        //Load Vocabulary file
        FileStorage fs;
        fs.open(vocabularFileName.toStdString(), FileStorage::READ);

        if (!fs.isOpened())
        {
            cout << "Failed to open " << vocabularFileName.toStdString() << endl;
            return;
        }

        fs["vocabulary"]>>vocabulary;

        if(vocabulary.empty()){
            cout<<"Error: Could not load Vocabulary "<<vocabularFileName.toStdString()  <<endl;
            return;
        }
        cout<<"Vocabulary loaded successfully - "<<vocabulary.rows<<","<<vocabulary.cols<<endl;
        fs.release();

        //////////////////////////// Creating Histogram databases for Training dataset //////////////////////////////////////////

        QDir dir(training_path);

        if (!dir.exists()){
            qWarning("Cannot find the training directory");
            return;
        }
        else
        {
            cout<<"Training dir opened"<<endl;

            dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

            //Create a nearest neighbor matcher
            Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher);
            Ptr<FeatureDetector> detectorFM=xfeatures2d::SiftFeatureDetector::create(0,3,0.08,10,1.6);
            //Ptr<FeatureDetector> detectorFM=xfeatures2d::SiftFeatureDetector::create();
            Ptr<DescriptorExtractor> extractorFM=xfeatures2d::SiftDescriptorExtractor::create();

            //Create BoF descriptor extractor
            BOWImgDescriptorExtractor bowDE(extractorFM,matcher);

            //Add vocabulary to BoF descriptor extractor
            bowDE.setVocabulary(vocabulary);

            vector<vector<int> > histDatabase;
            vector<string> imgIdDatabase;
            int x=0;


            //Loop through all training images
            clock_t start=std::clock();
#pragma omp parallel for
{
            foreach(QString dirFile, dir.entryList())
            {
                cout<<"Image: "<<x<<" File: "<<dirFile.toStdString()<<endl;
                QString filenameWithPath=dir.absolutePath()+"/"+dirFile;

                Mat matOriginal;
                vector<cv::KeyPoint> keypoints1;
                Mat descriptor1;
                Mat descriptorFM;
                std::vector<std::vector<int> >* pointIdxsOfClusters= new std::vector<std::vector<int> >;
                vector<int> imgHist;

                //Load Image
                matOriginal=cv::imread(filenameWithPath.toStdString(),CV_LOAD_IMAGE_GRAYSCALE);

                if(matOriginal.empty()){
                    cout<<" Image "<<dirFile.toStdString()<<" Could not be loaded"<<endl;
                    exit(1);
                }

                //Gaussian Smoothing
                Mat dst;
                cv::blur(matOriginal,dst,Size(3,3));

                //Binarization
                Mat thresImg;
                cv::threshold(dst,thresImg,127,255,CV_THRESH_BINARY);

                try{
                    //Detect Keypoints and extract descriptors
                    detectorFM->detect(thresImg,keypoints1);
                    extractorFM->compute(thresImg,keypoints1,descriptor1);
                    //Map to Clusters from vocabulary
                    bowDE.compute(descriptor1,descriptorFM,pointIdxsOfClusters);
                }
                catch(Exception e){
                    cout<<"Error: "<<e.msg<<endl;
                }

                QString imgNm=dirFile.remove(".png");

                //Compute histogram of the mapping to clusters
                //pointIdxsOfClusters contains list of (cluster# -> descriptors# mapped to it)
                for (std::vector<std::vector<int> >::iterator it1 = pointIdxsOfClusters->begin() ; it1 != pointIdxsOfClusters->end(); ++it1){
                    std::vector<int> row=*it1;
                    imgHist.push_back(row.size());
                }


#pragma omp critical(histdb)
                {
                    //Save Image identifer and corresponding histogram in master lists.
                    //Image identifier
                    imgIdDatabase.push_back(imgNm.toStdString());
                    //histogram
                    histDatabase.push_back(imgHist);
                }

                //release memory
                matOriginal.release();
                dst.release();
                thresImg.release();
                keypoints1.clear();
                descriptor1.release();
                descriptorFM.release();
                imgHist.clear();
                pointIdxsOfClusters->clear();

#pragma omp atomic
                x=x+1;
            }
}
            double duration = (( std::clock() - start ) / (double) CLOCKS_PER_SEC);
            cout<<"Time for histogram geneartion in seconds: "<< duration <<endl;

            cout<<"Histogram created for all training images"<<endl;

            cout<<"Saving histogram database"<<endl;
            QString histdbFileName=out_dir+"/"+QFileInfo(vocabularFileName).fileName().replace(QString("dictionary"),QString("histDatabase"));
            cout<<"Hist database file: "<<histdbFileName.toStdString()<<endl;


            //Save histograms generated for all images in disk
            FileStorage fs1(histdbFileName.toStdString(), FileStorage::WRITE);
            fs1<<"histDatabase";
            fs1<< "{";
            for (int i = 0; i < histDatabase.size(); i++)
            {
                fs1 << "histDatabase_" + QString::number(i).toStdString();
                vector<int> tmp = histDatabase[i];
                fs1 << tmp;
                tmp.clear();
            }

            fs1<< "}";

            fs1<<"imgIdDatabase";
            fs1<<"{";
            for (int i = 0; i < imgIdDatabase.size(); i++)
            {
                fs1 << "imgIdDatabase_" + QString::number(i).toStdString();
                string tmp = imgIdDatabase[i];
                fs1 << tmp;
                tmp.clear();
            }

            fs1<< "}";

            fs1.release();

            //release memory
            histDatabase.clear();
            imgIdDatabase.clear();
        }

    }



}

/////////////////////// on_CreateHistogramDatabase_clicked ////////////////////////////////
/// \brief MainWindow::on_CreateHistogramDatabase_clicked
/// \param none
/// \Description Prompts to choose vocabulary file, uploads it and creates histogram for
/// all training images
/// \return
//////////////////////////////////////////////////////////////////////////////////////////
/*
void MainWindow::on_CreateHistogramDatabase_clicked()
{
    //////////////////////////// Select Vocabulary file //////////////////////////////////////////
    QString strFileName= QFileDialog::getOpenFileName();

    if(strFileName== ""){
        cout<<"File not chosen"<<endl;
        ui->lblVocabulary->setText("Vocabulary file not chosen");
        return;
    }

    FileStorage fs;
    fs.open(strFileName.toStdString(), FileStorage::READ);

    if (!fs.isOpened())
    {
        cout << "Failed to open " << strFileName.toStdString() << endl;
        return;
    }
    //////////////////////////// Creating Histogram databases for Training dataset //////////////////////////////////////////
    Mat vocabulary;

    fs["vocabulary"]>>vocabulary;

    if(vocabulary.empty()){
        cout<<"Error: Could not load Vocabulary"<<endl;
        return;
    }
    cout<<"Vocabulary loaded successfully - "<<vocabulary.rows<<","<<vocabulary.cols<<endl;
    fs.release();

    QDir dir(training_path);

    if (!dir.exists()){
        qWarning("Cannot find the training directory");
        return;
    }
    else
    {
        cout<<"Training dir opened"<<endl;

        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        vector<cv::KeyPoint> keypoints1;
        Mat descriptor1;
        Mat descriptorFM;

        //create a nearest neighbor matcher
        Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher);

        Ptr<FeatureDetector> detectorFM=xfeatures2d::SiftFeatureDetector::create();
        Ptr<DescriptorExtractor> extractorFM=xfeatures2d::SiftDescriptorExtractor::create();

        //create BoF descriptor extractor
        BOWImgDescriptorExtractor bowDE(extractorFM,matcher);
        bowDE.setVocabulary(vocabulary);

        vector<vector<int> > histDatabase;
        vector<string> imgIdDatabase;

        int x=0;
        Mat matOriginal;
        std::vector<std::vector<int> >* pointIdxsOfClusters= new std::vector<std::vector<int> >;
        vector<int> imgHist;

        //Loop through all images in training directory
        foreach(QString dirFile, dir.entryList())
        {
             cout<<"Progress: "<<x<<" File: "<<dirFile.toStdString()<<endl;
             QString filenameWithPath=dir.absolutePath()+"/"+dirFile;
             cout<<"filenameWithPath....."<<filenameWithPath.toStdString()<<endl;
             matOriginal=cv::imread(filenameWithPath.toStdString(),CV_LOAD_IMAGE_GRAYSCALE);

             if(matOriginal.empty()){
                 cout<<" Image "<<dirFile.toStdString()<<" Could not be loaded"<<endl;
                 exit(1);
             }
             try{
                cout<<"CKPT1"<<endl;
                detectorFM->detect(matOriginal,keypoints1);
                cout<<"CKPT2: "<<keypoints1.size()<<endl;
                extractorFM->compute(matOriginal,keypoints1,descriptor1);
                cout<<"CKPT3:"<<descriptor1.rows<<","<<descriptor1.cols<<endl;
                bowDE.compute(descriptor1,descriptorFM,pointIdxsOfClusters);   //Alternative function - own implementation
             }
             catch(Exception e){
                     cout<<"Error: "<<e.msg<<endl;
             }
             QString imgNm=dirFile.remove(".png");
             cout<<imgNm.toStdString()<<endl;

             //Image identifier
             imgIdDatabase.push_back(imgNm.toStdString());
             //compute histogram for image
             for (std::vector<std::vector<int> >::iterator it1 = pointIdxsOfClusters->begin() ; it1 != pointIdxsOfClusters->end(); ++it1){
                 std::vector<int> row=*it1;
                 imgHist.push_back(row.size());
             }

             //histogram
             histDatabase.push_back(imgHist);

             //release memory
             matOriginal.release();
             keypoints1.clear();
             descriptor1.release();
             descriptorFM.release();
             imgHist.clear();
             pointIdxsOfClusters->clear();

             x=x+1;
        }

        cout<<"Histogram created for all training images"<<endl;
        cout<<"Saving histogram database"<<endl;

        QString histdbFileName=out_dir+"/"+QFileInfo(strFileName).fileName().replace(QString("dictionary"),QString("histDatabase"));
        cout<<"Hist database file: "<<histdbFileName.toStdString()<<endl;

        FileStorage fs1(histdbFileName.toStdString(), FileStorage::WRITE);
        fs1<<"histDatabase";
        fs1<< "{";
        for (int i = 0; i < histDatabase.size(); i++)
        {
            fs1 << "histDatabase_" + QString::number(i).toStdString();
            vector<int> tmp = histDatabase[i];
            fs1 << tmp;
        }

        fs1<< "}";

        fs1<<"imgIdDatabase";
        fs1<<"{";
        for (int i = 0; i < imgIdDatabase.size(); i++)
        {
            fs1 << "imgIdDatabase_" + QString::number(i).toStdString();
            string tmp = imgIdDatabase[i];
            fs1 << tmp;
        }

        fs1<< "}";

        fs1.release();

        //release memory
        histDatabase.clear();
        imgIdDatabase.clear();
    }
}
*/




//////////////// KMeans Clustering Implementation : Distance Metric (Cosine similarity) /////////////////
vector<vector<float> > MainWindow::kmeansClusterMultiDim(int k)
{
    cout<<"Clustering started"<<endl;
    int inputlen=featuresUnclustered.rows;
    vector<vector<float> > ccVec;
    vector<int> cc_index;
    vector<int>::iterator p;
    int r;
    vector<float> temp,clcenter,cc1;
    double dist=0,mindist=0;
    vector<double> distVect;

    temp.reserve(featuresUnclustered.row(0).cols);
    cout<<"Space allocated"<<endl;

    clock_t start = std::clock();

    //Random intialization of cluster centers
    for(int i = 0; i < k; ++i)
    {
       do
       {
          r = rand()%inputlen;
          p=std::find(cc_index.begin(),cc_index.end(),r);

       }while( p!=cc_index.end());

       cout<<r<<",";
       cc_index.push_back(r);
       featuresUnclustered.row(r).copyTo(temp);
       ccVec.push_back(temp);
       temp.clear();
    }
    cout<<endl;
    //release memory
    cc_index.clear();
    cc_index.shrink_to_fit();
    temp.shrink_to_fit();
/*
    //kmeans ++ initialization
    cout<<"Kmeans ++ Initialization"<<endl;

    //Select first random center
    srand(time(NULL));
    int r1 = rand()%inputlen;
    cout<<"First CC: "<<r1<<endl;
    featuresUnclustered.row(r1).copyTo(cc1);
    cc_index.push_back(r1);
    ccVec.push_back(cc1);

    clock_t start = std::clock();
    std::random_device rd;
    default_random_engine generator(rd());
    int total_comparison=0;

    for(int j=0;j<(k-1);j++){
        cout<<"No of Clusters: "<<j<<endl;

        distVect.clear();

        //calculate dist between chosen cluster centers and all points
        for(int i=0;i<featuresUnclustered.rows;i++)
        {
            featuresUnclustered.row(i).copyTo(temp);
            int idx=0;
            dist=0; mindist=0;

            //determine the nearest chosen center
            for(vector<vector<float> >::iterator it=ccVec.begin();it<ccVec.end();++it)
            {
                total_comparison++;
                clcenter=*it;
                if(std::equal(clcenter.begin(),clcenter.end(),temp.begin())){
                    mindist=0;
                }
                else{
                    dist=computeDist(clcenter,temp);

                    if(idx==0)
                        mindist=dist;
                    if(dist<mindist)
                        mindist=dist;

                    clcenter.clear();
                }
                idx++;
            }

            distVect.push_back(mindist);
            temp.clear();
        }

        //determine probability of being chosen as cluster center of each point - dist/sum_of_all_dist
        discrete_distribution<int> distribution(distVect.begin(),distVect.end());

        //Select next cluster center with the probabilty calculated.
        do
        {
            r = distribution(generator);
            p=std::find(cc_index.begin(),cc_index.end(),r);
        }while( p!=cc_index.end());

        cc_index.push_back(r);
        featuresUnclustered.row(r).copyTo(temp);
        ccVec.push_back(temp);
        temp.clear();
    }

    cout<<"Total comparisons: "<<total_comparison<<endl;
    cout<<endl<<"Clusters initialized"<<endl;
    double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;


//    for(vector<int>::iterator it=cc_index.begin();it<cc_index.end();it++)
//        cout<<*it<<",";

    cout<<endl;*/

    double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    cout<<"Time for initializing clusters in seconds: "<< duration <<endl<<endl;

    cout<<endl<<"Starting clustering"<<endl;

    vector<int> nnIdx_clCenter;
    int idx,minccIdx,iteration=0;
    bool terminated;
    vector<float> rowvec;
    dist=0; mindist=0;

    //Allocate memory
    clcenter.reserve(featuresUnclustered.row(0).cols);
    nnIdx_clCenter.reserve(featuresUnclustered.rows);
    rowvec.reserve(featuresUnclustered.row(0).cols);


    do
    {
        terminated=true;
        cout<<"Iteration : "<<iteration<<endl;
        iteration++;

        ////////////////////////////////// Find Nearest Cluster //////////////////////////////////

        cout<<"Finding nearest cluster"<<endl;

        nnIdx_clCenter.clear();

        //For each feature vector, find nearest cluster
        for(int i=0;i<featuresUnclustered.rows;i++)
        {
            if(i%10000==0)
                cout<<"Feature: "<<i<<endl;

            featuresUnclustered.row(i).copyTo(rowvec);

            idx=0;
            minccIdx=0;
            //cout<<"Clusters..... ";
            //Compute nearest cluster
            for(vector<vector<float> >::iterator it=ccVec.begin();it<ccVec.end();++it)
            {

                try{
                    clcenter=*it;
                }
                catch(Exception e)
                {
                    cout<<"Error: "<<e.msg<<endl;
                }

                //calculate distance - cosine similarity metric
                try{
                    dist=computeDist(rowvec,clcenter);
                }
                catch(Exception e)
                {
                    cout<<"Error: "<<e.msg<<endl;
                }

                if(idx==0)
                    mindist=dist;

                if(dist<mindist){
                    mindist=dist;
                    minccIdx=idx;
                }

                //cout<<idx<<" , ";
                idx++;

                //release memory
                clcenter.clear();

            }


            //Nearest Cluster center - Store Index minccIdx
            nnIdx_clCenter.push_back(minccIdx);
            rowvec.clear();
        }

        rowvec.shrink_to_fit();
        clcenter.shrink_to_fit();

        cout<<"Features mapping to nearest cluster centers completed"<<endl<<"nnIdx size: "<<nnIdx_clCenter.size()<<endl;

        ////////////////////////////// Calculate new cluster centers //////////////////////////////

        cout<<endl<<"Calculating New cluster centers"<<endl;

        int ccIdx=0;
        vector<vector<float> > cc_features;
        vector<vector<float> > newccVec;
        vector<float> newcc;

        cout<<"Cluster : ";

        //For each cluster center find mapped features from nnIdx_clCenter
        for(vector<vector<float> >::iterator it=ccVec.begin();it<ccVec.end();++it)
        {
            clcenter=*it;
            int inputIdx=0;
            vector<float> t;

            cout<<ccIdx<<" ";
            //Find features mapped to this cluster center => cc_features
            for(vector<int>::iterator it1=nnIdx_clCenter.begin();it1<nnIdx_clCenter.end();++it1)
            {
                int tempidx=*it1;

                if(ccIdx==tempidx)
                {
                    //if feature vector is mapped to this cluster center,
                    //push the feature to cc_features
                    try
                    {
                        featuresUnclustered.row(inputIdx).copyTo(t);
                    }
                    catch(Exception e)
                    {
                        cout<<"Error: "<<e.msg<<endl;
                        exit(1);
                    }

                    cc_features.push_back(t);
                    t.clear();
                }

                inputIdx++;
            }

            //compute new cluster center= weighted average of cc_features
            newcc=calcAvgVec(cc_features);
            newccVec.push_back(newcc);

            //Compare oldCenter and newCenter and set Termination Criterion Flag variable
            if(!std::equal(clcenter.begin(),clcenter.end(),newcc.begin())){
                terminated=false;
                cout<<"New cluster center is different "<<endl;
            }

            //release memory
            cc_features.clear();
            newcc.clear();

            ccIdx++;

            if(terminated)
                cout<<"Terminated: true"<<endl;
            else
                cout<<"Terminated: false"<<endl;
        }

        cout<<"New cluster centers generated for iteration "<<iteration<<endl;

        ccVec.clear();
        ccVec=newccVec;
        newccVec.clear();
        cout<<endl;
    }while(iteration<300 && !terminated);

    cout<<"Iteration: "<<iteration<<endl;
    return ccVec;
}


//////////////////////// Compute Average Vector ////////////////////////////
vector<float> MainWindow::calcAvgVec(vector<vector<float> > obj){
    vector<float> weights;
    vector<float> avgVec;
    float sum;
    vector<float> rvec;

    //Calculate weights
    for(int i=0;i<obj.size();i++)
    {
        try{
            rvec=obj[i];
        }
        catch(Exception e){
                cout<<"Error: "<<e.msg<<endl;
                exit(1);
        }

        sum=(std::accumulate(rvec.begin(), rvec.end(), 0));
        float temp=sum/rvec.size();
        weights.push_back(temp);
        rvec.clear();
    }

    //calculate weighted average
    for(int i=0;i<obj[0].size();i++)
    {
        sum=0;
        for(int j=0;j<obj.size();j++)
        {
            sum+=weights[j]*obj[j][i];
        }
        avgVec.push_back(sum/obj.size());
    }
    return avgVec;
}


void MainWindow::on_pushButton_2_clicked()
{

}
