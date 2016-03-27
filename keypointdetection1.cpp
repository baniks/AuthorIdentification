#include "keypointdetection1.h"
#include "ui_keypointdetection1.h"

KeypointDetection1::KeypointDetection1(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeypointDetection1)
{
    ui->setupUi(this);
}

KeypointDetection1::~KeypointDetection1()
{
    delete ui;
}
