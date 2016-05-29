#include "mainwindow.h"
#include <QApplication>
#include<keypointdetection.h>

QString training_path;
QString sample_imgs_path;
QString test_path;
QString out_dir;
QString form_file_path;
QString uniqWriterImgFile_path;
QString single_sample_list;
bool featureFlag;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Setting path variables
    training_path="/Users/macbook/Dataset/iam/training_data";
    sample_imgs_path="/Users/macbook/Dataset/iam/train_kp_data";    //(For keypointdetection.cpp)
    test_path="/Users/macbook/Dataset/iam/test_data";
    out_dir="/Users/macbook/AuthorIdentificationNew/out";
    form_file_path="/Users/macbook/AuthorIdentificationNew/verifydata/forms1.txt";
    uniqWriterImgFile_path="/Users/macbook/AuthorIdentificationNew/verifydata/uniq_writer_img_list.txt";
    single_sample_list="/Users/macbook/AuthorIdentificationNew/verifydata/single_sample_list.txt";

    //Setting flag -
    //  True: features need to be extracted during training phase
    //  False: Upload extracted feature file (.yml)
    featureFlag=true;

    //Show Training Window
    MainWindow w;
    w.show();

    //Uncomment below line in order to detect keypoints on sample images, located in path sample_imgs_path
    //to see the impact of variation of sigma and threshold during keypoint detection
    //KeypointDetection k;

    return a.exec();
}
