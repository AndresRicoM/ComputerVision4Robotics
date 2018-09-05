#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define CV_CAST_8U

using namespace std;
using namespace cv;

Mat auxImage;
Mat img;

void rgb2yiq(const Mat img) {

  uchar r, g, b;
  double y, i, q;

  Mat out_y(img.rows, img.cols, CV_8UC1);
  Mat out_i(img.rows, img.cols, CV_8UC1);
  Mat out_q(img.rows, img.cols, CV_8UC1);
  Mat out(img.rows, img.cols, CV_8UC3);

  /* convert image from RGB to YIQ */

  int m=0, n=0;
  for(m=0; m<img.rows; m++)
  {
    for(n=0; n<img.cols; n++)
    {
      r = img.data[m*img.step + n*3 + 2];
      g = img.data[m*img.step + n*3 + 1];
      b = img.data[m*img.step + n*3 ];
      y = 0.299*r + 0.587*g + 0.114*b;
      i = 0.596*r - 0.275*g - 0.321*b;
      q = 0.212*r - 0.523*g + 0.311*b;

      out_y.data[m*out_y.step+n] = y;
      out_i.data[m*out_i.step+n] = CV_CAST_8U((int)(i));
      out_q.data[m*out_q.step+n ] = CV_CAST_8U((int)(q));
      out.data[m*img.step+n*3 +2] = y;
      out.data[m*img.step+n*3 +1] = CV_CAST_8U((int)(i));
      out.data[m*img.step+n*3 ] = CV_CAST_8U((int)(q));

    }
  }

  auxImage = out;

}

int main(int argc, char *argv[]) {
  VideoCapture camera = VideoCapture(0);
  bool isCameraAvailable = camera.isOpened();

  while(true) {
    if (isCameraAvailable) {
      camera.read(img);
    }

    rgb2yiq(img);

    imshow("Converted Image", auxImage);

    char key = waitKey(1);
    if(key == 'x' || key == 27 ){ // 27 = ESC
      break;
    }
  }
}
