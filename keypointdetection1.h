#ifndef KEYPOINTDETECTION1_H
#define KEYPOINTDETECTION1_H

#include <QDialog>

namespace Ui {
class KeypointDetection1;
}

class KeypointDetection1 : public QDialog
{
    Q_OBJECT

public:
    explicit KeypointDetection1(QWidget *parent = 0);
    ~KeypointDetection1();

private:
    Ui::KeypointDetection1 *ui;
};

#endif // KEYPOINTDETECTION1_H
