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

using namespace std;
using namespace cv;


RNG rng(12345);

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

void MainWindow::on_pushButton_clicked()
{
    //////////////////////////// Preprocess Training Images - One Time /////////////////////////////
    //preprocessIAM();
    cout<<"Preprocessing completed"<<endl;

    //////////////////////////// File Dialog and Cleaning //////////////////////////////////////////

    QStringList files = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more files to open",
                            ".",
                            "Images (*.png *.xpm *.jpg)");
    QStringList list = files;

    if(files.size()== 0){
            ui->lblChosenDir->setText("Image not chosen");
            return;
        }

    QString fileErrorMsg="Error: List of images not loaded - ";
    cout<<"Number of files chosen - "<<list.count()<<endl;

    vector<cv::KeyPoint> keypoints;
    Mat descriptor;
    Mat featuresUnclustered;
    int cnt=0;
    int totalKeypt=0;
    cv::Mat matOriginal;
    cv::Mat matGrayscale;

    //Loop through all images in training directory
    for (QStringList::Iterator it = list.begin();it != list.end(); ++it) {

         QString filename=*it;

         //Load Image
         matOriginal=cv::imread(filename.toStdString());

         if(matOriginal.empty()){
             fileErrorMsg.append("Image "+filename);
         }
         else{


             cv::cvtColor(matOriginal,matGrayscale,CV_BGR2GRAY);
             cout<<"Input img size "<<matGrayscale.rows<<","<<matGrayscale.cols<<endl;

             ////////////////// Image Preprocessing ///////////////////////////

             //Removing shadow on left and fixed heading
             //Mat croppedImage=preprocessIAM(matGrayscale,filename);

             //Saving cropped image
             QStringList filenamePieces= filename.split("/");

             ////////////////// Feature Extraction ///////////////////////////
             //Mat kpImg;

             //Detect Key Points
             Ptr<cv::xfeatures2d::SiftFeatureDetector> detector=xfeatures2d::SiftFeatureDetector::create();
             detector->detect(matGrayscale,keypoints);
             //cout<<"Cropped image size: "<<matGrayscale.size()<<endl;
             cout<<"Number of keypoints detected: "<<keypoints.size()<<endl;
             //cout<<"Keypoint 1 coordinates:"<<keypoints[1].pt<<" Keypoint 1 angle:"<<keypoints[1].angle<<" Keypoint 1 scale/octave:"<<keypoints[1].octave<<endl;

             //for average keypoints per img
             totalKeypt+=keypoints.size();
             cnt++;

             //Saving Keypoint Image
             //cv::drawKeypoints(matGrayscale,keypoints,kpImg,Scalar::all(-1),cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
             //string kpFilename="tempKP_"+filenamePieces.value( filenamePieces.length() - 1 ).toStdString();
             //cv::imwrite(kpFilename,kpImg);

             //Computing Descriptors for Each Keypoint
             Ptr<cv::xfeatures2d::SiftDescriptorExtractor> extractor=xfeatures2d::SiftDescriptorExtractor::create();
             extractor->compute(matGrayscale, keypoints,descriptor);
             //cout<<"Descriptor size:"<<descriptor.rows<<" x "<<descriptor.cols<<endl<<"Descriptor: "<<descriptor.row(1)<<endl;

             //put feature descriptors for all images in a single Mat object
             featuresUnclustered.push_back(descriptor);
             cout<<"Progress: "<<cnt<<endl;

             //release memory
             detector.release();
             extractor.release();
         }

         //release memory
         matOriginal.release();
         matGrayscale.release();
         descriptor.release();
         keypoints.clear();

    }


    cout<<"Average number of keypoints per image: "<<(double)totalKeypt/cnt<<endl;
    cout<<"Total number of unclustered Descriptors : "<<featuresUnclustered.size()<<endl<<endl;

    //////////////////////////// Clustering and Vocabulary Creation //////////////////////////////////////////
    //Construct BOWKMeansTrainer
    int clusterCenter=400;
    cv::TermCriteria tc(CV_TERMCRIT_ITER,100,0.001);
    int retries=1;
    int flags=cv::KMEANS_PP_CENTERS; //for Kmeans++

    BOWKMeansTrainer bowTrainer(clusterCenter,tc,retries,flags);
    bowTrainer.add(featuresUnclustered);

    //cluster the feature vectors
    Mat vocabulary=bowTrainer.cluster();

    cout<<"Vocabulary generated"<<endl;
    cout<<"Vocabulary size:"<<vocabulary.size()<<endl;

    //store the vocabulary
    FileStorage fs("dictionary_400.yml", FileStorage::WRITE);
    fs << "vocabulary" << vocabulary;
    fs.release();
    cout<<"Vocabulary written to disk"<<endl;
    cout<<"Click on 'Create Histogram Database' Button "<<endl;

    //release memory
    bowTrainer.clear();
    featuresUnclustered.release();

}

/////////////////////// preprocess ////////////////////////////////
/// \brief MainWindow::preprocess
/// \param inputImage, filename
/// \return
/////////////////////////////////////////////////////////////////

void MainWindow::preprocessIAM(){

    QDir dir("/home/student/Dataset/iam/train_data");
    if (!dir.exists())
        qWarning("Cannot find the example directory");
    else
    {
        cout<<"dir exists"<<endl;

        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        foreach(QString dirFile, dir.entryList())
        {
             cout<<"File name: "<<dirFile.toStdString()<<" Dir path: "<<dir.absolutePath().toStdString()<<endl;
             QString filenameWithPath=dir.absolutePath()+"/"+dirFile;

             Mat img=cv::imread(filenameWithPath.toStdString());
             cout<<"Image loaded - "<<img.rows<<", "<<img.cols<<endl;

             //Removing shadow on left and fixed heading
             Mat tmpImg=img(Rect(100,370,(img.cols-100),(img.rows-370)));

             //Check if the image file is in single sample list
             QFile checkListFile("/home/student/Desktop/single_sample_list.txt");

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
                 croppedImage=cropImage(tmpImg);
                 //save to file
                 cv::imwrite(filenameWithPath.toStdString(),croppedImage);
             }
             else
             {
                 cout<<"single sample file: "<<dirFile.toStdString()<<endl;
                 cout<<"Cropped image size: "<<croppedImage.rows<<" "<<croppedImage.cols<<endl;

                 //Single sample files - crop and split
                 croppedImage=cropImage(tmpImg);
                 Mat part1_trainimg=croppedImage(Rect(0,0,croppedImage.cols,(croppedImage.rows/2)));
                 cout<<"part1: "<<croppedImage.rows/2<<" OK"<<endl;
                 Mat part2_testimg=croppedImage(Rect(0,(croppedImage.rows/2)+1,croppedImage.cols,(croppedImage.rows/2)-1));
                 //save to file
                 cv::imwrite(filenameWithPath.toStdString(),part1_trainimg);
                 string tstFileName="/home/student/Dataset/iam/test_data/"+dirFile.toStdString();
                 cv::imwrite(tstFileName,part2_testimg);
             }
        }
        cout<<"All files processed"<<endl;
     }

}

/////////////////////// cropImage ////////////////////////////////
/// \brief OriginalImg::cropImage
/// \param inputImage
/// \return
/////////////////////////////////////////////////////////////////


cv::Mat MainWindow::cropImage(cv::Mat inputImage){
    cv::Mat cannyOutput;
    vector<Vec2f> lines;
    vector<int> rows;
    cv::Mat croppedImg,temp;

    cv::Canny(inputImage,cannyOutput,127,255,3);
    //cvtColor(cannyOutput, cdst, CV_GRAY2BGR);

    cv::HoughLines(cannyOutput,lines,1, CV_PI/180, 700, 0, 0);
    if(lines.size()==0)
    {
        //return default locations
        cout<<"cropping at default location"<<endl;
        temp=inputImage(Rect(0,280,inputImage.cols,2180));
        temp.copyTo(croppedImg);
    }
    else{
        for( size_t i = 0; i < lines.size(); i++ )
          {
             float rho = lines[i][0], theta = lines[i][1];
             if ( theta>1.56 and theta<1.58)
             {
                 double b = sin(theta);
                 double y0 = b*rho;
                 rows.push_back(y0);

                 //Comment below when not needed
/*                 double x1,y1,x2,y2;
                 x1 = int(x0 + 1000*(-b));
                 y1 = int(y0 + 1000*(a));

                 x2 = int(x0 - 1000*(-b));
                 y2 = int(y0 - 1000*(a));
                 line(cdst,Point(x1,y1),Point(x2,y2),(0,0,255),2);  */
             }
          }


        if(rows.size()>0)
        {
            double bucket_size=50;
            int number_of_buckets=(int)(inputImage.rows/bucket_size);
            vector<int> histogram(number_of_buckets);

            //Create histogram of the rows where horizontal lines are detected
            for(vector<int>::iterator it=rows.begin();it != rows.end(); ++it)
            {
                int row=*it;
                int bucket=(int)(row/bucket_size);
                histogram[bucket]+=1;
            }

            int nonzero_cnt=histogram.size()-count(histogram.begin(),histogram.end(),0);

            //histogram Non zero count size >=2 return last 2 entry
            if(nonzero_cnt>=2)
            {
                int flag=0;
                int hlines[2];
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

                temp=inputImage(Rect(0,hlines[0],inputImage.cols,(hlines[1]-hlines[0])));
                temp.copyTo(croppedImg);
            }
            else{
                //return default locations
                cout<<"cropping at default location"<<endl;
                temp=inputImage(Rect(0,280,inputImage.cols,2180));
                temp.copyTo(croppedImg);
            }
        }


    }

    return croppedImg;

    // Get contour with maximum area

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
   // return cv::Mat(inputImage,bounding_rect);
}

void MainWindow::on_CreateHistogramDatabase_clicked()
{

    //////////////////////////// Creating databases for Training dataset //////////////////////////////////////////7
    Mat vocabulary;

    FileStorage fs;
    fs.open("dictionary_400.yml", FileStorage::READ);

    fs["vocabulary"]>>vocabulary;

    if(vocabulary.empty()){
        cout<<"Error: Could not load Vocabulary"<<endl;
        return;
    }
    cout<<"Vocabulary loaded successfully"<<endl;
    QDir dir("/home/student/Dataset/iam/train_data");

    if (!dir.exists()){
        qWarning("Cannot find the training directory");
        return;
    }
    else
    {
        cout<<"Test dir opened"<<endl;

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
        Mat matOriginal,img;
        std::vector<std::vector<int> >* pointIdxsOfClusters= new std::vector<std::vector<int> >;
        vector<int> imgHist;

        //Loop through all images in training directory
        foreach(QString dirFile, dir.entryList())
        {
             cout<<"Progress: "<<x<<endl;
             QString filenameWithPath=dir.absolutePath()+"/"+dirFile;

             matOriginal=cv::imread(filenameWithPath.toStdString());
             cv::cvtColor(matOriginal,img,CV_BGR2GRAY);

             detectorFM->detect(img,keypoints1);
             extractorFM->compute(img,keypoints1,descriptor1);

             //Alternative function - own implementation
             bowDE.compute(descriptor1,descriptorFM,pointIdxsOfClusters);
             QString imgNm=dirFile.remove(".png");

             cout<<imgNm.toStdString()<<endl;

             //push Image identifier
             imgIdDatabase.push_back(imgNm.toStdString());
             //compute histogram for image
             for (std::vector<std::vector<int> >::iterator it1 = pointIdxsOfClusters->begin() ; it1 != pointIdxsOfClusters->end(); ++it1){
                 std::vector<int> row=*it1;
                 imgHist.push_back(row.size());
             }

             histDatabase.push_back(imgHist);
             x=x+1;

             //release memory
             try{
                 matOriginal.release();
                 img.release();
                 keypoints1.clear();
                 descriptor1.release();
                 descriptorFM.release();
                 imgHist.clear();
                 pointIdxsOfClusters->clear();
             }
             catch(Exception e){
                 cout<<"Error: "<<e.msg<<endl;
                 return;
             }
        }

        cout<<"Histogram created for all training images"<<endl;
        cout<<"Saving histogram database"<<endl;

        FileStorage fs1("histDatabase_400.yml", FileStorage::WRITE);
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
