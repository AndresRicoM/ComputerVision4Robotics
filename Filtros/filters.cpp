#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace cv;

int DELAY_CAPTION = 1500;
int DELAY_BLUR = 100;
int MAX_KERNEL_LENGTH = 21;

Point anchor;
double delta;
int ddepth;
int kernel_size;
//char* window_name = "filter2D Demo";
int c;
int lowthres = 500;

int erosion_elem = 1;
int erosion_size = 0;
int dilation_elem = 0;
int dilation_size = 0;
int const max_elem = 2;
int const max_kernel_size = 21;
bool frozen = false;

Mat img; Mat gauss; Mat ave; Mat med; Mat lap; Mat dx; Mat gray; Mat avek; Mat edges; Mat aux;
Mat hat; Mat absdx; Mat absdy; Mat dy; Mat eros; Mat dil; Mat enhan;

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

  imshow("Original Image", img);

  //Morfological Filters
  cvtColor( img, gray, CV_BGR2GRAY );
  Mat binaryMat(gray.size(), gray.type());
  threshold(gray, gray, 100, 255, cv::THRESH_BINARY);
  erode(gray, eros,Mat(), Point(-1, -1), 2, 1, 1);
  dilate(gray, dil, Mat(), Point(-1, -1), 2, 1, 1);
  imshow( "Filtro Dilatador", eros );
  imshow("Filtro Erosion", dil);

  //Sobel Derivatives
  cvtColor(img, gray, COLOR_BGR2GRAY);
  Sobel(gray, dx, CV_16S ,1,0,3,1,0,BORDER_DEFAULT);
  convertScaleAbs(dx, absdx);
  imshow("Derivativa en X", absdx);

  cvtColor(img, gray, COLOR_BGR2GRAY);
  Sobel(gray, dy, CV_16S ,0,1,3,1,0,BORDER_DEFAULT);
  convertScaleAbs(dy, absdy);
  imshow("Derivativa en Y", absdy);

  //Hat Filter
  GaussianBlur( img, hat, Size( 11, 11 ), 5, 5); //Kernell Size 11 x 11
  cvtColor(hat, gray, COLOR_BGR2GRAY);
  Laplacian(gray, hat, CV_16S,5, 1, 0, BORDER_DEFAULT);
  convertScaleAbs(hat, hat);
  imshow("Filtro de Sombrero Mexicano", hat);

  //Edge Detection Filter
  cvtColor(img, gray, COLOR_BGR2GRAY);
  Canny(gray, edges, lowthres, (lowthres*3), 5);
  aux = Scalar::all(0);
  edges.copyTo(aux, edges);
  imshow("Detector de Bordes", aux);

  //Enhancement Filter
  cvtColor(img, gray, COLOR_BGR2GRAY);
  Laplacian(gray, enhan, CV_16S,3, 1, 0, BORDER_DEFAULT);
  convertScaleAbs(enhan, enhan);
  imshow("Filtro Enfatizador", enhan);

  //Laplace Filter
  cvtColor(img, gray, COLOR_BGR2GRAY);
  Laplacian(gray, lap, CV_16S,5, 1, 0, BORDER_DEFAULT);
  convertScaleAbs(lap, lap);
  imshow("Filtro Laplaciano", lap);

  //Average Filter
  anchor = Point( -1, -1 );
  delta = 0;
  ddepth = -1;
  kernel_size = 11;
  avek =  Mat::ones( kernel_size, kernel_size, CV_32F )/ (float)(kernel_size*kernel_size);
  filter2D(img, ave, ddepth , avek, anchor, delta, BORDER_DEFAULT );
  imshow("Filtro Promedio", ave );

  ///Gaussian Filter
  GaussianBlur( img, gauss, Size( 11, 11 ), 5, 5); //Kernell Size 11 x 11
  imshow("Filtro Gaussiaenno", gauss);

  //Median Filter
  medianBlur ( img, med, 7 );
  imshow("Median Filter", med);

    char key = waitKey(1);
    if(key == 'x' || key == 27 ){ // 27 = ESC
      break;
    }
  }
}
