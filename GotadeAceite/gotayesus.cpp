#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <deque>

using namespace std;
using namespace cv;


int main(int argc, char *argv[]) {

  Mat gray = imread("image.png",CV_LOAD_IMAGE_GRAYSCALE);
  threshold(gray, gray, 80, 255, cv::THRESH_BINARY);
  
  Mat aux = Mat::zeros(gray.rows, gray.cols, CV_64FC1);

  imshow("origimage", gray);
    imshow("origaux", aux);


    int ccol = 300; //Valores de coordenadas para comenzar en el centro de la imagen.
    int crow = 200;

    //int pixelValue = gray.at<uchar>(crow,ccol);

    Vec3b pixelValue = gray.at<Vec3b>(ccol, crow);
    cout << pixelValue;
    //Fo : list
    deque<int> fox;
    deque<int> foy;

    if (pixelValue[0] == 255) {
      //Agregar Semilla a Lista. (Row, Column, Flag)
      //Fo = {Ṕ}
      fox.push_back(ccol);
      foy.push_back(crow);
    }

    //q[0,3]:Vector
    int vectorposx[4] = { 1,0,-1,0 };
    int vectorposy[4] = { 0,1,0,-1 };
    int auxposx;
    int auxposy;
    int gvalue;
    int auxvalue;
    
    while (!fox.empty() && !foy.empty()) { //Mientras la lista tenga elementos.
      //Obtener Elemento de la Lista
      //Asignar row y col a crow y crow.
      crow = fox.front();
      fox.pop_front();
      ccol = foy.front();
      foy.pop_front();

      for(int i=0; i<4; i++){

        auxposx = crow + vectorposx[i];
        auxposy = ccol + vectorposy[i];
        //cout <<"----" << i << "----"<< endl;
        //cout << auxposx << endl;
        //cout << auxposy << endl;

        if (auxposx>0 && auxposx<gray.rows && auxposy>0 && auxposy<gray.cols){

        Vec3b gvalue = gray.at<Vec3b>(auxposx, auxposy);
        //cout << gvalue << "gvalue"<<endl;

        auxvalue = aux.at<uchar>(auxposx, auxposy);

        //cout << auxvalue <<"aux";
        //cout << endl;



        if ( gvalue[0] == 255 && auxvalue!=255){
          //cout <<"entro! " << auxposx<<","<<auxposy<<endl;
          aux.at<uchar>(auxposx, auxposy) = 255;
          fox.push_back(auxposx);
          foy.push_back(auxposy);
        }
        //else{
          //cout <<"no entro! " << auxposx<<","<<auxposy<<endl;
        //}
      }
      }  

    imshow("image", gray);
    imshow("aux", aux);
    
    //} 
    }waitKey(0);
