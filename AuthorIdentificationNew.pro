#-------------------------------------------------
#
# Project created by QtCreator 2016-01-25T23:08:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AuthorIdentificationNew
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    testwindow.cpp \
    keypointdetection.cpp

HEADERS  += mainwindow.h \
    testwindow.h \
    keypointdetection.h

FORMS    += mainwindow.ui \
    testwindow.ui

INCLUDEPATH +=/usr/local/include/

LIBS+=-L/usr/local/lib -fopenmp -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_dnn -lopencv_dpm -lopencv_line_descriptor -lopencv_reg -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_rgbd -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_text -lopencv_face -lopencv_xfeatures2d -lopencv_calib3d -lopencv_features2d -lopencv_shape -lopencv_video -lopencv_flann -lopencv_xobjdetect -lopencv_objdetect -lopencv_ml -lopencv_xphoto -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core
