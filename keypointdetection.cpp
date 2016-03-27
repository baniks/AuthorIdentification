/***********************************************************************
* FILENAME :        keypointdetection.cpp
*
* DESCRIPTION :
*       Independent class implementation in order to see the impact of
*       variation of different parameters on SIFT keypoint detection.
*
* FUNCTIONS :
*       void     detectKeypoint1( )
*
*
************************************************************************/

#include "keypointdetection.h"
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
#include <array>
#include<math.h>

using namespace std;
using namespace cv;

extern QString out_dir;
extern QString sample_imgs_path;

KeypointDetection::KeypointDetection()
{
    cout<<"Keypoint Detection invoked"<<endl;
    detectKeypoint1();
    cout<<"Keypoint Detection completed"<<endl;
}

/***************************************************************************
* NAME :            void detectKeypoint1()
*
* DESCRIPTION :     detects SIFT keypoints
*
* INPUTS :
*       PARAMETERS:
*           None
*       GLOBALS :
*           QString     out_dir             path of out directory
*           QString     sample_imgs_path    path of sample images
* OUTPUTS :
*       RETURN :
*           None
* PROCESS :
*                   [1]  For all sample images
*                   [2]     Load Image in Grayscale
*                   [3]     Smooth
*                   [4]     Threshold
*                   [5]     For range of threshold and sigma values
*                   [6]         Detect SIFT keypoints
*                   [7]         Draw Keypoints on Image and Save to disk
*                   [8]         Display Image
*
****************************************************************************/

void KeypointDetection::detectKeypoint1(){

    QDir dir(sample_imgs_path);
    Mat matOriginal,matGrayscale;
    vector<cv::KeyPoint> keypoints;
    Mat kpImg;
    int cnt=0;

    if (!dir.exists())
        qWarning("Cannot find the example directory");
    else
    {
        cout<<"Dir opened"<<endl;

        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        //Loop through all files in the directory
        foreach(QString dirFile, dir.entryList())
        {
            cout<<"Image: "<<cnt<<endl;
            cnt++;
            QString filenameWithPath=dir.absolutePath()+"/"+dirFile;

            //Load Image in Grayscale
            matOriginal=cv::imread(filenameWithPath.toStdString());
            cv::cvtColor(matOriginal,matGrayscale,CV_BGR2GRAY);

            //Smoothing
            Mat dst;
            cv::blur(matGrayscale,dst,Size(3,3));

            //Binarization
            Mat thresImg;
            cv::threshold(dst,thresImg,127,255,CV_THRESH_BINARY);

            //Save thresholded image
            QString threshFN=out_dir+"/thres127_"+dirFile;
            cv::imwrite(threshFN.toStdString(),thresImg);


            //Vary threshold
            array<double,5> contrast_thresh={0.04,0.06,0.08,0.1,0.2};

            for(int i=0;i<contrast_thresh.size();i++){

                    cout<<"Contrast Threshold: "<<contrast_thresh[i]<<endl;

                    //Detect SIFT keypoints
                    Ptr<FeatureDetector> detectorFM=xfeatures2d::SiftFeatureDetector::create(0,3,contrast_thresh[i],10,1.6);
                    detectorFM->detect(thresImg,keypoints);
                    cout<<"Number of keypoints detected: "<<keypoints.size()<<endl;

                    //Draw detected keypoints and save image
                    cv::drawKeypoints(thresImg,keypoints,kpImg,Scalar::all(-1),cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
                    QString kpFilename=out_dir+"/tempKPSM_CT_"+QString::number(contrast_thresh[i])+"_"+dirFile;
                    cv::imwrite(kpFilename.toStdString(),kpImg);

                    //Show Image
                    Mat scaledkpImg;
                    cv::resize(kpImg,scaledkpImg,Size(),0.33,0.33,INTER_LINEAR);
                    QString winName="CT_"+QString::number(contrast_thresh[i])+dirFile;
                    namedWindow(winName.toStdString(),WINDOW_NORMAL);
                    cv::imshow(winName.toStdString(),scaledkpImg);

                    //release memory
                    kpImg.release();
                    detectorFM.release();
                }

            //Vary sigma
            array<double,5> sigma_values={1.6,1.8,2.0,2.2,2.4};    //0.5,0.8,1.0,1.2,1.4,1.6,1.8,2.0,2.2,2.4

            for(int i=0;i<sigma_values.size();i++){

                    cout<<"Sigma: "<<sigma_values[i]<<endl;

                    //Detect SIFT keypoints
                    Ptr<FeatureDetector> detectorFM=xfeatures2d::SiftFeatureDetector::create(0,3,0.04,10,sigma_values[i]);
                    detectorFM->detect(thresImg,keypoints);
                    cout<<"Number of keypoints detected: "<<keypoints.size()<<endl;

                    //Draw detected keypoints and save image
                    cv::drawKeypoints(thresImg,keypoints,kpImg,Scalar::all(-1),cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
                    QString kpFilename=out_dir+"/tempKPSM_sigma_"+QString::number(sigma_values[i])+"_"+dirFile;
                    cv::imwrite(kpFilename.toStdString(),kpImg);

                    //Show Image
                    Mat scaledkpImg;
                    cv::resize(kpImg,scaledkpImg,Size(),0.33,0.33,INTER_LINEAR);
                    QString winName="Sigma_"+QString::number(contrast_thresh[i])+dirFile;
                    namedWindow(winName.toStdString(),WINDOW_NORMAL);
                    cv::imshow(winName.toStdString(),scaledkpImg);

                    //release memory
                    kpImg.release();
                    detectorFM.release();
                }

            }
            waitKey(0);
            cv::destroyAllWindows();

            //release memory
            matOriginal.release();
            matGrayscale.release();

    }

}


