#include "mainwindow.h"
#include <QApplication>
#include<keypointdetection.h>

QString training_path;
QString sample_imgs_path;
QString test_path;
QString out_dir;
QString form_file_path;
QString uniqWriterImgFile_path;
bool featureFlag;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Setting path variables
    training_path="/Users/macbook/Dataset/iam/training_data";
    sample_imgs_path="/Users/macbook/Dataset/iam/train_kp_data";    //(For keypointdetection.cpp)
    test_path="/Users/macbook/Dataset/iam/test_data";
    out_dir="/Users/macbook/Workspace/build-AuthorIdentificationNew-Desktop_Qt_5_5_1_clang_64bit-Debug/out";
    form_file_path="/Users/macbook/Workspace/build-AuthorIdentificationNew-Desktop_Qt_5_5_1_clang_64bit-Debug/forms1.txt";
    uniqWriterImgFile_path="/Users/macbook/Workspace/build-AuthorIdentificationNew-Desktop_Qt_5_5_1_clang_64bit-Debug/uniq_writer_img_list.txt";

    //Setting flag -
    //  True: features need to be extracted during training phase
    //  False: Upload extracted feature file (.yml)
    featureFlag=false;

    //Show Training Window
    MainWindow w;
    w.show();

    //Uncomment below line in order to detect keypoints on sample images, located in path sample_imgs_path
    //to see the impact of variation of sigma and threshold during keypoint detection
    //KeypointDetection k;

    return a.exec();
}
