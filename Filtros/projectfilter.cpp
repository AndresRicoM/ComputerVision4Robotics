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

Mat final;

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

  cvtColor(img, final, COLOR_BGR2GRAY);
  Mat binaryMat(final.size(), final.type());
  threshold(final, final, 225, 255, cv::THRESH_BINARY);
  dilate(final, final, Mat(), Point(-1, -1), 2, 1, 1);
  erode(final, final,Mat(), Point(-1, -1), 2, 1, 1);
  imshow("Imagen Final", final);
  }
}
