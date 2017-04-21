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
        { 0, -1, 0},
        { -1, 4, -1},
        { 0, -1, 0}};
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

Mat MainWindow::medianFilter(Mat src)
{
    int window[9];

    Mat dst = src.clone();
    for(int y = 0; y < src.rows; y++)
        for(int x = 0; x < src.cols; x++)
            dst.at<uchar>(y,x) = 0.0;

    for(int y = 1; y < src.rows - 1; y++){
        for(int x = 1; x < src.cols - 1; x++){

            // Pick up window element
            window[0] = src.at<uchar>(y - 1 ,x - 1);
            window[1] = src.at<uchar>(y, x - 1);
            window[2] = src.at<uchar>(y + 1, x - 1);
            window[3] = src.at<uchar>(y - 1, x);
            window[4] = src.at<uchar>(y, x);
            window[5] = src.at<uchar>(y + 1, x);
            window[6] = src.at<uchar>(y - 1, x + 1);
            window[7] = src.at<uchar>(y, x + 1);
            window[8] = src.at<uchar>(y + 1, x + 1);

            // sort the window to find median
            int temp, i , j;
            for(i = 0; i < 9; i++){
                temp = window[i];
                for(j = i-1; j >= 0 && temp < window[j]; j--){
                    window[j+1] = window[j];
                }
                window[j+1] = temp;
            }

            // assign the median to centered element of the matrix
            dst.at<uchar>(y,x) = window[0];
        }
    }
    return dst;
}


// ------------------ Fin Fontions Utiles ----------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);

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
    QString fileName = QFileDialog::getOpenFileName(this,"Open Image File","/Users/chelalimohamed-tayeb/Pictures");

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
    /*
         Parameters:
            ksize – Size of the filter returned.
            sigma – Standard deviation of the gaussian envelope.
            theta – Orientation of the normal to the parallel stripes of a Gabor function.
            lambd – Wavelength of the sinusoidal factor.
            gamma – Spatial aspect ratio.
            psi – Phase offset.
            ktype – Type of filter coefficients. It can be CV_32F or CV_64F .

    */
    Mat gaborKernel = getGaborKernel(Size(3, 3),  10, 45, 50, 0.1);
    Mat gaborKernel1 = getGaborKernel(Size(3, 3),  10, 135, 50, 0.1);
    Mat im, im1;

    filter2D(gray, im, CV_8U, gaborKernel);
    filter2D(gray, im1, CV_8U, gaborKernel);

    imagee = Mat::zeros(im.rows, im.cols, CV_8UC1);
    for(int i=0;i<im.rows;i++){
        for(int j=0;j<im.cols;j++){
            imagee.at<uchar>(i,j) = (((int) im.at<uchar>(i,j)) + ((int) im1.at<uchar>(i,j)))/2;
        }
    }

    // Appiliquer un seuillage d'OTSU
    threshold(imagee, imagee, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);


    // Faire une dilatation
    int dilation_size = 10;
    Mat element = getStructuringElement( MORPH_RECT,
                                         Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                         Point( dilation_size, dilation_size ) );
    dilate(imagee, imagee, element);


    /*int erode_size = 3;
    Mat element1 = getStructuringElement( MORPH_RECT,
                                          Size( 2*erode_size + 1, 2*erode_size+1 ),
                                          Point( erode_size, erode_size ) );
    erode(imagee, imagee, element1);
    imagee = this->medianFilter(imagee);*/
    imagee = this->medianFilter(imagee);

    result = Mat::zeros(imagee.rows, imagee.cols, CV_8UC1);
    for(int i=0;i<imagee.rows;i++){
        for(int j=0;j<imagee.cols;j++){
            if( (int) object.at<uchar>(i,j) != 0 ){
                result.at<uchar>(i,j) = 0;
            }else{
                result.at<uchar>(i,j) = (((int) imagee.at<uchar>(i,j)) * ((int) gray.at<uchar>(i,j)));
            }
        }
    }
    threshold(result, result, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);

    image = new QImage(result.data, result.cols, result.rows, result.step, QImage::Format_Indexed8);
    item = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    scene2->addItem(item);

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



    threshold(object, object, 110, 255, CV_THRESH_OTSU);

    erode(object, object, element1);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(object, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

    if(!contours.empty()){
        //for every contour of the same hierarchy level
        for(int i = 0; i >= 0; i = hierarchy[i][0])
        {
            /*for(vector<Point>::iterator it = contours[i].begin(); it != contours[i].end(); ++it){
                //*it = *it + Point(50, 50);
                qDebug() << len(*it);
            }*/
            Rect rect = boundingRect(contours[i]);
            rect.x = rect.x-20;
            rect.y = rect.y-20;
            rect.height = rect.height+30;
            rect.width = rect.width+30;
            rectangle(object, rect, cv::Scalar(255), CV_FILLED);
        }
    }


    image = new QImage(object.data, object.cols, object.rows, object.step, QImage::Format_Indexed8);
    item = new QGraphicsPixmapItem(QPixmap::fromImage(*image));
    scene3->addItem(item);

}





