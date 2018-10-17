#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace cv;

Mat img;
Mat gray;
Mat aux;
bool frozen = false;

int main(int argc, char *argv[]) {
  VideoCapture camera = VideoCapture(0);
  bool isCameraAvailable = camera.isOpened();

  while(true) {
    if (!frozen) {
      camera.read(img);
    }

    switch(waitKey(3)){
				case 'f':
					frozen = not frozen;
					break;
    }

    cvtColor( img, gray, CV_BGR2GRAY );
    Mat binaryMat(gray.size(), gray.type());
    threshold(gray, gray, 80, 255, cv::THRESH_BINARY);
    aux = Mat::zeros(gray.rows, gray.cols, CV_64F);

    int ccol = 300; //Valores de coordenadas para comenzar en el centro de la imagen.
    int crow = 200;

    int pixelValue = gray.at<uchar>(crow,ccol);

    if (pixelValue == 0) {
      //Agregar Semilla a Lista. (Row, Column, Flag)
    }
    /*

    while (true) { //Mientras la lista tenga elementos.
      //Obtener Elemento de la Lista
      //Asignar row y col a crow y crow.
      //crow =
      //ccol =

      if ((gray.at<uchar>(crow , ccol + 1) == 0) && (aux.at<uchar>(crow , ccol + 1) == 0)) { //&& Not Flagged //Elemento de la derecha.
        //Agregar elemento a la lista.
        aux.at<uchar>(crow , ccol + 1) = 255;
      }

      if ((gray.at<uchar>(crow - 1 , ccol) == 0) && (aux.at<uchar>(crow - 1 , ccol) == 0)) { //&& Not Flagged //Elemento de arriba.
        //Agregar Elemento a lista.
        aux.at<uchar>(crow + 1 , ccol) = 255;
      }

      if ((gray.at<uchar>(crow , ccol -1) == 0) && (aux.at<uchar>(crow , ccol -1) == 0)) {
        //Agregar Elemento a la lista.
        aux.at<uchar>(crow , ccol -1) = 255;
      }

      if ((gray.at<uchar>(crow - 1 , ccol) == 0) && (aux.at<uchar>(crow , ccol -1) == 0)) {
        //Agregar Elemento a la lista.
        aux.at<uchar>(crow - 1 , ccol) = 255;
      }

    } */

    imshow("Image", gray);
    imshow("Mask", aux);

    char key = waitKey(1);
    if(key == 'x' || key == 27 ){ // 27 = ESC
      break;
    }
  }
}
