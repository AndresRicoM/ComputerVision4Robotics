#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <deque>

#include <typeinfo>
using namespace std;
using namespace cv;

Mat gray;
Mat aux;
Mat aux2;

int main(int argc, char *argv[]) {

  gray = imread("image.jpg",CV_LOAD_IMAGE_GRAYSCALE);
  threshold(gray, gray, 80, 255, cv::THRESH_BINARY);
  
  aux = Mat::zeros(gray.rows, gray.cols, CV_32F);

  aux2 = Mat::zeros(gray.rows, gray.cols, CV_8UC3);

  //cout << typeid(gray).name();
  cout << (int)gray.at<uchar>(10,10) <<endl;
  cout << aux.at<float>(10,10)<<endl;




  int crow;
  int ccol;
  int auxposx;
  int auxposy;
  int grayval;
  int auxval;
  
  //Fo: List
  deque<int> fox;
  deque<int> foy;

  //q[0,3]:Vector
  int vectorposx[4] = { 1,0,-1,0 };
  int vectorposy[4] = { 0,1,0,-1 };


  // Fo = {P}
  crow = 218;
  ccol = 496;
  fox.push_back(crow);
  foy.push_back(ccol);


  while (!fox.empty() && !foy.empty()) {

  	//Get Next, siguiente de la lista, Remove
  	crow = fox.front();
    fox.pop_front();
    ccol = foy.front();
    foy.pop_front();

    //For i:=0,3, i++; ciclo de vecinos
    for(int i=0; i<4; i++){

    	auxposx = crow + vectorposx[i];
        auxposy = ccol + vectorposy[i];

        if (auxposx>0 && auxposx<gray.rows && auxposy>0 && auxposy<gray.cols){


        grayval = (int)gray.at<uchar>( auxposx , auxposy );
        //auxval = aux.at<int>(auxposx , ccol);
        auxval = aux.at<float>( auxposx , auxposy );

        if (grayval==0 && auxval==0){

        	//cout << "entro" <<auxposx <<auxposy <<endl;
        	//para cambiar el BGR de la imagen
        	aux2.at<Vec3b>(auxposx, auxposy)[0] = 0;
        	aux2.at<Vec3b>(auxposx, auxposy)[1] = 255;
        	aux2.at<Vec3b>(auxposx, auxposy)[2] = 0;
        	//mascara de un solo bit 
        	aux.at<float>( auxposx , auxposy ) = 255;

        	fox.push_back(auxposx);
        	foy.push_back(auxposy);
        }
        else{
        	//cout << "no entro" <<auxposx <<auxposy <<endl;
        	continue;
        }

    	}
    }



  }

        

  	imshow("origimage", gray);
  	imshow("origaux", aux);
  	imshow("origaux2", aux2);
  	waitKey(0);


}
