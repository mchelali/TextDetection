#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <vector>
#include "qgraphicviewzoom.h"


// ------------------ Fontions Utiles ----------------------------------
Mat MainWindow::convolution(Mat input, int mask[3][3])
{
    Mat conv = Mat::zeros(input.rows, input.cols, CV_8UC1);
    for(int i=1; i<input.rows-1 ;i++){
        for(int j=1; j<input.cols-1 ;j++){
            int pixel=0;
            for(int x=-1; x<2;x++){
                for(int y=-1;y<2;y++){
                    pixel = pixel + ((int) input.at<uchar>(i+x,j+y)) * mask[x+1][y+1];
                }
            }
            conv.at<char>(i,j) = abs(pixel);
        }
    }
    return conv;
}

Mat MainWindow::calcConteur(Mat img)
{
    Mat cont = Mat::zeros(img.rows, img.cols, CV_8UC1);

    int mask[3][3]= {
        { 1, 1, 1},
        { 1,-8, 1},
        { 1, 1, 1}};
    cont = convolution(img, mask);

    return cont;
}

Mat MainWindow::filtreMoyenneur(Mat img)
{
    Mat cont = Mat::zeros(img.rows, img.cols, CV_8UC1);

    int mask[3][3]= {
        { 1/9, 1/9, 1/9},
        { 1/9, 1/9, 1/9},
        { 1/9, 1/9, 1/9}} ;
    cont = convolution(img, mask);

    return cont;
}
// ------------------ Fin Fontions Utiles ----------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene();
    scene2 = new QGraphicsScene();
    ui->graphicsView_2->setScene(scene2);
    scene3 = new QGraphicsScene();
    ui->graphicsView_3->setScene(scene3);

    QGraphicViewZoom *z = new QGraphicViewZoom(ui->graphicsView);
    z->set_modifiers(Qt::NoModifier);

    QGraphicViewZoom *z1 = new QGraphicViewZoom(ui->graphicsView_2);
    z1->set_modifiers(Qt::NoModifier);

    QGraphicViewZoom *z2 = new QGraphicViewZoom(ui->graphicsView_3);
    z2->set_modifiers(Qt::NoModifier);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionClose_triggered()
{
    close();
}

void MainWindow::on_actionupload_image_triggered()
{
    /*
     * Cette fonction permet de charger une image
     */
    scene->clear();
    QString fileName = QFileDialog::getOpenFileName(this,"Open Image File","/Users/");

    if(!fileName.isEmpty()){
        img = imread(fileName.toStdString());

        if(img.empty()){
            QMessageBox::information(this,"Image Viewer","Error Displaying image");
            return;
        }
        cvtColor(img, img, CV_BGR2RGB);
        cvtColor(img, gray, CV_RGB2GRAY);

        image = new QImage(img.data, img.cols, img.rows,img.step , QImage::Format_RGB888);
        item = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
        scene->addItem(item);
        ui->graphicsView->setScene(scene);
        ui->graphicsView->fitInView(scene->sceneRect(),Qt::KeepAspectRatio);
        ui->graphicsView->show();
    }

}

void MainWindow::on_detecte_clicked()
{
    /*
     * Detectio du text
     */
    scene2->clear();
    Mat imagee;

    Mat inputBin;
    threshold(gray, inputBin, 0, 255, CV_THRESH_OTSU);

    //equalizeHist(gray, gray);



    //Appiliquer un filtre de Gabor
    Mat gaborKernel = getGaborKernel(Size(5, 5),  50, 45, 5, 0.50);
    filter2D(gray, imagee, CV_8U, gaborKernel);

    // Appiliquer un seuillage d'OTSU
    threshold(imagee, imagee, 0, 255, CV_THRESH_OTSU);


    // Faire une dilatation
    int dilation_size = 15;
    Mat element = getStructuringElement( MORPH_RECT,
                                        Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                        Point( dilation_size, dilation_size ) );
    dilate(imagee, imagee, element);


    result = Mat::zeros(imagee.rows, imagee.cols, CV_8UC1);
    for(int i=0;i<imagee.rows;i++){
        for(int j=0;j<imagee.cols;j++){
            result.at<uchar>(i,j) =  ((int) imagee.at<uchar>(i,j)) * ((int) gray.at<uchar>(i,j));
        }
    }

    image = new QImage(result.data, result.cols, result.rows, result.step, QImage::Format_Indexed8);
    item = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    scene2->addItem(item);

    //ui->graphicsView_2->setScene(scene2);
    ui->graphicsView_2->fitInView(scene2->sceneRect(), Qt::KeepAspectRatio);
    ui->graphicsView_2->show();
}

void MainWindow::on_decObject_clicked()
{
    scene3->clear();

    Mat contour;
    contour = this->calcConteur(gray);

    int dilation_size = 2;
    Mat element = getStructuringElement( MORPH_RECT,
                                         Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                         Point( dilation_size, dilation_size ) );
    dilate(contour, contour, element);

    int erode_size = 8;
    Mat element1 = getStructuringElement( MORPH_RECT,
                                          Size( 2*erode_size + 1, 2*erode_size+1 ),
                                          Point( erode_size, erode_size ) );
    erode(contour, object, element1);

    /*Mat conv = Mat::zeros(object.rows, object.cols, CV_8UC1);
    for(int i=1; i<object.rows-1 ;i++){
        for(int j=1; j<object.cols-1 ;j++){
            float pixel=0;
            for(int x=-1; x<2;x++){
                for(int y=-1;y<2;y++){
                    pixel = pixel + ((int) object.at<uchar>(i+x,j+y));
                }
            }
            conv.at<char>(i,j) = abs(pixel/9);
        }
    }

    object = conv;*/
    image = new QImage(object.data, object.cols, object.rows, object.step, QImage::Format_Indexed8);
    item = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    scene3->addItem(item);

    ui->graphicsView_3->fitInView(scene3->sceneRect(), Qt::KeepAspectRatio);
    ui->graphicsView_3->show();

}





