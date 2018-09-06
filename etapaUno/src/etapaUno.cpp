/*
 * Title: Window Click Example
 * Class: Vision para Robot
 * Instructor: Dr. Jose Luis Gordillo (http://robvis.mty.itesm.mx/~gordillo/)
 * Code: Manlio Barajas (manlito@gmail.com)
 * Institution: Tec de Monterrey, Campus Monterrey
 * Date: January 28, 2013
 *
 * Description: Shows the most basic interaction with OpenCV HighGui window.
 *
 * This programs uses OpenCV http://www.opencv.org/
 */

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <opencv/cv.h>

 #define CV_CAST_8U

using namespace cv;
using namespace std;

// Here we will store points
vector<Point> points;
vector<Point> histPoints;

// Store values from image
//vector<> values

// Frozen picture
bool frozen = false, closeP=false;

int trackValue = 0;

int low_H = 0, low_S = 0, low_V = 0;
int high_H = 255, high_S = 255, high_V = 255;
int low_R = 0, low_G = 0, low_B = 0;
int high_R = 255, high_G = 255, high_B = 255;
int low_Y = 0, low_I = 0, low_Q = 0;
int high_Y = 255, high_I = 255, high_Q = 255;

/* Create images where captured and transformed frames are going to be stored */
	Mat auxImage;
	Mat hsv_thres;
	Mat bgr_thres;
	Mat yiq_thres;
	Mat output;
	Mat GUI;


	// Create image template for coloring
	Mat solidColor(50, 50, CV_8UC3);

	/* Create images where captured and transformed frames are going to be stored */
	Mat currentImage;

// Store BGR values of a point in image
Vec3b channels;

Vec3i minRange;
Vec3i maxRange;

/* This is the callback that will only display mouse coordinates */
void mouseCoordinatesExampleCallback(int event, int x, int y, int flags, void* param);

void bgrHistogram();
void hsvHistogram();
void yiqHistogram();


// Function for conversion between RGB and YIQ spaces
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
  out.copyTo(auxImage);
}


void modThreshold(int,void*)
{
	Mat tempImage;
	threshold( auxImage, tempImage, trackValue, 255, 0);
	imshow("B&W", tempImage);
}

void mod_HSV_threshold(int,void*)
{
	cvtColor(currentImage, auxImage, cv::COLOR_BGR2HSV);
	inRange(auxImage, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), hsv_thres);
	currentImage.copyTo(output, hsv_thres);
	imshow("HSV", hsv_thres);
	imshow("HSV Threshold", output);
	output.release();
}

void mod_BGR_threshold(int,void*)
{
	inRange(currentImage, Scalar(low_B, low_G, low_R), Scalar(high_B, high_G, high_R), bgr_thres);
	currentImage.copyTo(output, bgr_thres);
	imshow("RGB", bgr_thres);
	imshow("RGB Threshold", output);
	output.release();
}

void mod_YIQ_threshold(int,void*)
{
  rgb2yiq(currentImage);
	inRange(auxImage, Scalar(low_Y, low_I, low_Q), Scalar(high_Y, high_I, high_Q), yiq_thres);
	currentImage.copyTo(output, yiq_thres);
	imshow("YIQ", yiq_thres);
	imshow("YIQ Threshold", output);
	output.release();
}

// Function to autoset ranges for thresholds in color spaces
void getParameterRange(){
	if(!(getWindowProperty("RGB", WND_PROP_AUTOSIZE) == -1)){
		// Reset ranges for search
		minRange = {255,255,255};
		maxRange = {0,0,0};
		// Scan image in range of selection
		for(int i = 0; i<points[1].y-points[0].y; i++){
			for(int j = 0; j<points[1].x-points[0].x; j++){
				channels = currentImage.at<Vec3b>(points[0].y+i,points[0].x+j);
				// Search for minimum and maximum
				for(int k = 0; k<3; k++){
					if (minRange[k]>channels[k]){
						minRange[k]=channels[k];
					}
					if (maxRange[k]<channels[k]){
						maxRange[k]=channels[k];
					}
				}
			}
		}
		setTrackbarPos("Bmin", "RGB", minRange[0]);
		setTrackbarPos("Gmin", "RGB", minRange[1]);
		setTrackbarPos("Rmin", "RGB", minRange[2]);
		setTrackbarPos("Bmax", "RGB", maxRange[0]);
		setTrackbarPos("Gmax", "RGB", maxRange[1]);
		setTrackbarPos("Rmax", "RGB", maxRange[2]);
	}else if(!(getWindowProperty("HSV", WND_PROP_AUTOSIZE) == -1)){
		// Reset ranges for search
		minRange = {255,255,255};
		maxRange = {0,0,0};
		// Scan image in range of selection
		for(int i = 0; i<points[1].y-points[0].y; i++){
			for(int j = 0; j<points[1].x-points[0].x; j++){
				channels = auxImage.at<Vec3b>(points[0].y+i,points[0].x+j);
				// Search for minimum and maximum
				for(int k = 0; k<3; k++){
					if (minRange[k]>channels[k]){
						minRange[k]=channels[k];
					}
					if (maxRange[k]<channels[k]){
						maxRange[k]=channels[k];
					}
				}
			}
		}
		setTrackbarPos("Hmin", "HSV", minRange[0]-1);
		setTrackbarPos("Smin", "HSV", minRange[1]-30);
		setTrackbarPos("Vmin", "HSV", minRange[2]-25);
		setTrackbarPos("Hmax", "HSV", maxRange[0]+1);
		setTrackbarPos("Smax", "HSV", maxRange[1]+30);
		setTrackbarPos("Vmax", "HSV", maxRange[2]+25);
	}else if(!(getWindowProperty("YIQ", WND_PROP_AUTOSIZE) == -1)){
		// Reset ranges for search
		minRange = {255,255,255};
		maxRange = {0,0,0};
		// Scan image in range of selection
		for(int i = 0; i<points[1].y-points[0].y; i++){
			for(int j = 0; j<points[1].x-points[0].x; j++){
				channels = auxImage.at<Vec3b>(points[0].y+i,points[0].x+j);
				// Search for minimum and maximum
				for(int k = 0; k<3; k++){
					if (minRange[k]>channels[k]){
						minRange[k]=channels[k];
					}
					if (maxRange[k]<channels[k]){
						maxRange[k]=channels[k];
					}
				}
			}
		}
		setTrackbarPos("Ymin", "YIQ", minRange[0]);
		setTrackbarPos("Imin", "YIQ", minRange[1]);
		setTrackbarPos("Qmin", "YIQ", minRange[2]);
		setTrackbarPos("Ymax", "YIQ", maxRange[0]);
		setTrackbarPos("Imax", "YIQ", maxRange[1]);
		setTrackbarPos("Qmax", "YIQ", maxRange[2]);
	}

}


int main(int argc, char *argv[])
{
	/* First, open camera device */
	VideoCapture camera;
    camera.open(0);


    /* Create main OpenCV window to attach callbacks */
    namedWindow("Image");
    setMouseCallback("Image", mouseCoordinatesExampleCallback);

    histPoints.push_back(Point(0,0));


    while (true)
	{
		if (!frozen){
			/* Obtain a new frame from camera */
			camera >> currentImage;
		}

		if (currentImage.data)
		{
			// copy contents from camera to graphic user interface image
			currentImage.copyTo(GUI);

			// paint range selection rectangle on screen (GUI)
			if (points.size() > 1) {
				rectangle(GUI, (Point)points[0], (Point)points[1], Scalar( 0, 255, 0 ), 1, 8, 0);
			}


			/* Show GUI on main window */
			imshow("Image", GUI);
      if(!frozen){
        if(!(getWindowProperty("RGB", WND_PROP_AUTOSIZE) == -1)){
          mod_BGR_threshold(0,0);
          bgrHistogram();
        }
        if(!(getWindowProperty("HSV", WND_PROP_AUTOSIZE) == -1)){
          mod_HSV_threshold(0,0);
          hsvHistogram();
        }
        if(!(getWindowProperty("YIQ", WND_PROP_AUTOSIZE) == -1)){
          mod_YIQ_threshold(0,0);
          yiqHistogram();
        }
      }else{
        if(!(getWindowProperty("RGB", WND_PROP_AUTOSIZE) == -1)){
          bgrHistogram();
        }
        if(!(getWindowProperty("HSV", WND_PROP_AUTOSIZE) == -1)){
          hsvHistogram();
        }
        if(!(getWindowProperty("YIQ", WND_PROP_AUTOSIZE) == -1)){
          yiqHistogram();
        }
      }


			// Keypress actions
			switch(waitKey(3)){
				case 'f':
					/* If 'f' is pressed, freeze image */
					frozen = not frozen;
					break;
				case 'b':
					/* If 'b' is pressed, b&w tools */
					cvtColor(currentImage, auxImage, cv::COLOR_BGR2GRAY);
					imshow("B&W", auxImage);
					createTrackbar("Umbral", "B&W", &trackValue, 255, modThreshold);
					break;
				case 'r':
					/* If 'r' is pressed, rgb tools */
					bgrHistogram();
					imshow("RGB", currentImage);
					createTrackbar("Bmin", "RGB", &low_B, 255, mod_BGR_threshold);
					createTrackbar("Bmax", "RGB", &high_B, 255, mod_BGR_threshold);
					createTrackbar("Gmin", "RGB", &low_G, 255, mod_BGR_threshold);
					createTrackbar("Gmax", "RGB", &high_G, 255, mod_BGR_threshold);
					createTrackbar("Rmin", "RGB", &low_R, 255, mod_BGR_threshold);
					createTrackbar("Rmax", "RGB", &high_R, 255, mod_BGR_threshold);
					mod_BGR_threshold(0,0);
					break;
				case 'h':
					/* If 'h' is pressed, hsv tools */
					imshow("HSV", currentImage);
					createTrackbar("Hmin", "HSV", &low_H, 255, mod_HSV_threshold);
					createTrackbar("Hmax", "HSV", &high_H, 255, mod_HSV_threshold);
					createTrackbar("Smin", "HSV", &low_S, 255, mod_HSV_threshold);
					createTrackbar("Smax", "HSV", &high_S, 255, mod_HSV_threshold);
					createTrackbar("Vmin", "HSV", &low_V, 255, mod_HSV_threshold);
					createTrackbar("Vmax", "HSV", &high_V, 255, mod_HSV_threshold);
					mod_HSV_threshold(0,0);
          hsvHistogram();
					break;
				case 'y':
						/* If 'y' is pressed, yiq tools */
						imshow("YIQ", currentImage);
						createTrackbar("Ymin", "YIQ", &low_Y, 255, mod_YIQ_threshold);
						createTrackbar("Ymax", "YIQ", &high_Y, 255, mod_YIQ_threshold);
						createTrackbar("Imin", "YIQ", &low_I, 255, mod_YIQ_threshold);
						createTrackbar("Imax", "YIQ", &high_I, 255, mod_YIQ_threshold);
						createTrackbar("Qmin", "YIQ", &low_Q, 255, mod_YIQ_threshold);
						createTrackbar("Qmax", "YIQ", &high_Q, 255, mod_YIQ_threshold);
						mod_YIQ_threshold(0,0);
            yiqHistogram();
						break;
				case 'x':
					/* If 'x' is pressed, exit program */
					closeP = true;
					break;
			}
			if (closeP) break;


		}
		else
		{
			cout << "No image data.. " << endl;
		}
	}
}


void mouseCoordinatesExampleCallback(int event, int x, int y, int flags, void* param)
{
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
						// Print mouse position
            cout << "  Mouse X, Y: " << x << ", " << y << " ---> ";


						// Get rgb values at point
						channels = currentImage.at<Vec3b>(y,x);
						//Open new window with solid color of pixel at click
						imshow("Color", solidColor.setTo(Scalar(int(channels[0]), int(channels[1]), int(channels[2]))));
            // Print color space values at point
            if(!(getWindowProperty("HSV", WND_PROP_AUTOSIZE) == -1)){
              Vec3b auxChannels = auxImage.at<Vec3b>(y,x);
              cout << "H: " << int(auxChannels[0])<< " S: " << int(auxChannels[1])<< " V: " << int(auxChannels[2])<<endl;
            }else if(!(getWindowProperty("YIQ", WND_PROP_AUTOSIZE) == -1)){
              Vec3b auxChannels = auxImage.at<Vec3b>(y,x);
              cout << "Y: " << int(auxChannels[0])<< " I: " << int(auxChannels[1])<< " Q: " << int(auxChannels[2])<<endl;
            }else{
              cout << "B: " << int(channels[0])<< " G: " << int(channels[1])<< " R: " << int(channels[2])<<endl;
            }

            /*  Store a point */
						points.push_back(Point(x, y));
            histPoints.clear();
            histPoints.push_back(Point(x,y));

            break;
        case CV_EVENT_MOUSEMOVE:
						// Update mouse position on click-drag
						if (points.size() > 0){
							if (points.size() > 1)points.pop_back();
							points.push_back(Point(x, y));
						}
            break;
        case CV_EVENT_LBUTTONUP:
						// Call function to auto set color spaces ranges
						getParameterRange();
						points.clear();
            break;
    }
}

void bgrHistogram(){
	/// Separate the image in 3 places ( B, G and R )
  vector<Mat> bgr_planes;
  split( currentImage, bgr_planes );

  /// Establish the number of bins
  int histSize = 256;

  /// Set the ranges ( for B,G,R) )
  float range[] = { 0, 256 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  Mat b_hist, g_hist, r_hist;

  /// Compute the histograms:
  calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

  // Draw the histograms for B, G and R
  int hist_w = 512; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImageB( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	Mat histImageG( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	Mat histImageR( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

  /// Normalize the result to [ 0, histImage.rows ]
  normalize(b_hist, b_hist, 0, histImageB.rows, NORM_MINMAX, -1, Mat() );
  normalize(g_hist, g_hist, 0, histImageB.rows, NORM_MINMAX, -1, Mat() );
  normalize(r_hist, r_hist, 0, histImageB.rows, NORM_MINMAX, -1, Mat() );

  /// Draw for each channel
  for( int i = 1; i < histSize; i++ )
  {
      line( histImageB, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );
      line( histImageG, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                       Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImageR, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                       Scalar( 0, 0, 255), 2, 8, 0  );
  }

  // Draw point values on histograms as lines
  Vec3b chan = currentImage.at<Vec3b>(histPoints[0].y, histPoints[0].x);

  float b = float(chan[0])*(float(hist_w)/255);
  float g = float(chan[1])*(float(hist_w)/255);
  float r = float(chan[2])*(float(hist_w)/255);

  line(histImageB, Point(int(b), 0), Point(int(b), hist_h), Scalar( 255, 255, 255 ), 1, 8, 0);
  line(histImageG, Point(int(g), 0), Point(int(g), hist_h), Scalar( 255, 255, 255 ), 1, 8, 0);
  line(histImageR, Point(int(r), 0), Point(int(r), hist_h), Scalar( 255, 255, 255 ), 1, 8, 0);

  /// Display
  namedWindow("Histogram B", CV_WINDOW_AUTOSIZE );
  imshow("Histogram B", histImageB );
	namedWindow("Histogram G", CV_WINDOW_AUTOSIZE );
  imshow("Histogram G", histImageG );
	namedWindow("Histogram R", CV_WINDOW_AUTOSIZE );
  imshow("Histogram R", histImageR );

  moveWindow("Histogram B", 0, 600);
  moveWindow("Histogram G", 600, 600);
  moveWindow("Histogram R", 1200, 600);
}

void hsvHistogram(){
	/// Separate the image in 3 places ( H, S and V )
  vector<Mat> hsv_planes;
  split( auxImage, hsv_planes );

  /// Establish the number of bins
  int histSize = 256;

  /// Set the ranges ( for H,S,V) )
  float range[] = { 0, 256 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  Mat h_hist, s_hist, v_hist;

  /// Compute the histograms:
  calcHist( &hsv_planes[0], 1, 0, Mat(), h_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &hsv_planes[1], 1, 0, Mat(), s_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &hsv_planes[2], 1, 0, Mat(), v_hist, 1, &histSize, &histRange, uniform, accumulate );

  // Draw the histograms for H, S and V
  int hist_w = 512; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImageH( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	Mat histImageS( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	Mat histImageV( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

  /// Normalize the result to [ 0, histImage.rows ]
  normalize(h_hist, h_hist, 0, histImageH.rows, NORM_MINMAX, -1, Mat() );
  normalize(s_hist, s_hist, 0, histImageH.rows, NORM_MINMAX, -1, Mat() );
  normalize(v_hist, v_hist, 0, histImageH.rows, NORM_MINMAX, -1, Mat() );

  /// Draw for each channel
  for( int i = 1; i < histSize; i++ )
  {
      line( histImageH, Point( bin_w*(i-1), hist_h - cvRound(h_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(h_hist.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );
      line( histImageS, Point( bin_w*(i-1), hist_h - cvRound(s_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(s_hist.at<float>(i)) ),
                       Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImageV, Point( bin_w*(i-1), hist_h - cvRound(v_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(v_hist.at<float>(i)) ),
                       Scalar( 0, 0, 255), 2, 8, 0  );
  }

  // Draw point values on histograms as lines
  Vec3b chan = auxImage.at<Vec3b>(histPoints[0].y, histPoints[0].x);

  float h = float(chan[0])*(float(hist_w)/255);
  float s = float(chan[1])*(float(hist_w)/255);
  float v = float(chan[2])*(float(hist_w)/255);

  line(histImageH, Point(int(h), 0), Point(int(h), hist_h), Scalar( 255, 255, 255 ), 1, 8, 0);
  line(histImageS, Point(int(s), 0), Point(int(s), hist_h), Scalar( 255, 255, 255 ), 1, 8, 0);
  line(histImageV, Point(int(v), 0), Point(int(v), hist_h), Scalar( 255, 255, 255 ), 1, 8, 0);

  /// Display
  namedWindow("Histogram H", CV_WINDOW_AUTOSIZE );
  imshow("Histogram H", histImageH );
	namedWindow("Histogram S", CV_WINDOW_AUTOSIZE );
  imshow("Histogram S", histImageS );
	namedWindow("Histogram V", CV_WINDOW_AUTOSIZE );
  imshow("Histogram V", histImageV );

  moveWindow("Histogram H", 0, 600);
  moveWindow("Histogram S", 600, 600);
  moveWindow("Histogram V", 1200, 600);
}

void yiqHistogram(){
	/// Separate the image in 3 places ( Y, I and Q )
  vector<Mat> yiq_planes;
  split( auxImage, yiq_planes );

  /// Establish the number of bins
  int histSize = 256;

  /// Set the ranges ( for H,S,V) )
  float range[] = { 0, 256 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  Mat y_hist, i_hist, q_hist;

  /// Compute the histograms:
  calcHist( &yiq_planes[0], 1, 0, Mat(), y_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &yiq_planes[1], 1, 0, Mat(), i_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &yiq_planes[2], 1, 0, Mat(), q_hist, 1, &histSize, &histRange, uniform, accumulate );

  // Draw the histograms for H, S and V
  int hist_w = 512; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImageY( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	Mat histImageI( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	Mat histImageQ( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

  /// Normalize the result to [ 0, histImage.rows ]
  normalize(y_hist, y_hist, 0, histImageY.rows, NORM_MINMAX, -1, Mat() );
  normalize(i_hist, i_hist, 0, histImageY.rows, NORM_MINMAX, -1, Mat() );
  normalize(q_hist, q_hist, 0, histImageY.rows, NORM_MINMAX, -1, Mat() );

  /// Draw for each channel
  for( int i = 1; i < histSize; i++ )
  {
      line( histImageY, Point( bin_w*(i-1), hist_h - cvRound(y_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(y_hist.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );
      line( histImageI, Point( bin_w*(i-1), hist_h - cvRound(i_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(i_hist.at<float>(i)) ),
                       Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImageQ, Point( bin_w*(i-1), hist_h - cvRound(q_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(q_hist.at<float>(i)) ),
                       Scalar( 0, 0, 255), 2, 8, 0  );
  }

  // Draw point values on histograms as lines
  Vec3b chan = auxImage.at<Vec3b>(histPoints[0].y, histPoints[0].x);

  float y = float(chan[0])*(float(hist_w)/255);
  float i = float(chan[1])*(float(hist_w)/255);
  float q = float(chan[2])*(float(hist_w)/255);

  line(histImageY, Point(int(y), 0), Point(int(y), hist_h), Scalar( 255, 255, 255 ), 1, 8, 0);
  line(histImageI, Point(int(i), 0), Point(int(i), hist_h), Scalar( 255, 255, 255 ), 1, 8, 0);
  line(histImageQ, Point(int(q), 0), Point(int(q), hist_h), Scalar( 255, 255, 255 ), 1, 8, 0);

  /// Display
  namedWindow("Histogram Y", CV_WINDOW_AUTOSIZE );
  imshow("Histogram Y", histImageY );
	namedWindow("Histogram I", CV_WINDOW_AUTOSIZE );
  imshow("Histogram I", histImageI );
	namedWindow("Histogram Q", CV_WINDOW_AUTOSIZE );
  imshow("Histogram Q", histImageQ );

  moveWindow("Histogram Y", 0, 600);
  moveWindow("Histogram I", 600, 600);
  moveWindow("Histogram Q", 1200, 600);
}
