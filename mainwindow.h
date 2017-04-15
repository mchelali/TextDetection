#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;



/*
 * ui->graphicview ---------------->  represente l'image originale
 * ui->graphicview_2 ---------------->  represente l'image qui conteient que du text
 * ui->graphicview_3 ---------------->  represente l'image qui contient que les image
*/


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionClose_triggered();

    void on_actionupload_image_triggered();

    void on_detecte_clicked();

    void on_decObject_clicked();

private:
    Mat convolution(Mat input, int mask[3][3] );
    Mat calcConteur(Mat img);
    Mat filtreMoyenneur(Mat img);

    Ui::MainWindow *ui;
    Mat img;
    Mat gray;
    Mat object;
    Mat result;

    QImage *image;
    QGraphicsScene *scene;
    QGraphicsScene *scene2;
    QGraphicsScene *scene3;
    QGraphicsPixmapItem *item;

};

#endif // MAINWINDOW_H
