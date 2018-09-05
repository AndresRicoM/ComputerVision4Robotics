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

using namespace cv;
using namespace std;

// Here we will store points
vector<Point> points;

// Frozen picture
bool frozen = false, closeP=false;

int trackValue = 0;

int low_H = 0, low_S = 0, low_V = 0;
int high_H = 255, high_S = 255, high_V = 255;
int low_R = 0, low_G = 0, low_B = 0;
int high_R = 255, high_G = 255, high_B = 255;

/* Create images where captured and transformed frames are going to be stored */
	Mat auxImage;
	Mat hsv_thres;
	Mat bgr_thres;
	Mat output;

	// Create image template for coloring
	Mat solidColor(50, 50, CV_8UC3);

	/* Create images where captured and transformed frames are going to be stored */
	Mat currentImage;

// Store BGR values of a point in image
Vec3b colorsBGR;

/* This is the callback that will only display mouse coordinates */
void mouseCoordinatesExampleCallback(int event, int x, int y, int flags, void* param);
void bgrHistogram();





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



int main(int argc, char *argv[])
{
	/* First, open camera device */
	VideoCapture camera;
    camera.open(0);


    /* Create main OpenCV window to attach callbacks */
    namedWindow("Image");
    setMouseCallback("Image", mouseCoordinatesExampleCallback);


    while (true)
	{
		if (!frozen){
			/* Obtain a new frame from camera */
			camera >> currentImage;
		}

		if (currentImage.data)
		{

			/* Draw all points */
			for (int i = 0; i < points.size(); ++i) {
				circle(currentImage, (Point)points[i], 2, Scalar( 255, 0, 0 ), CV_FILLED);
			}
			if (points.size() > 1) {
				for (int p =1; p < points.size(); p++){
					line(currentImage, (Point)points[p-1], (Point)points[p], Scalar( 0, 255, 0 ), 1, 8, 0);
				}
			}


			/* Show image */
			imshow("Image", currentImage);

			switch(waitKey(3)){
				case 'f':
					/* If 'f' is pressed, freeze image */
					frozen = not frozen;
					break;
				case 'b':
					/* If 'b' is pressed, b&w image */
					cvtColor(currentImage, auxImage, cv::COLOR_BGR2GRAY);
					imshow("B&W", auxImage);
					createTrackbar("Umbral", "B&W", &trackValue, 255, modThreshold);
					break;
				case 'r':
					/* If 'r' is pressed, rgb image */
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
					/* If 'h' is pressed, hsv image */
					imshow("HSV", currentImage);
					createTrackbar("Hmin", "HSV", &low_H, 255, mod_HSV_threshold);
					createTrackbar("Hmax", "HSV", &high_H, 255, mod_HSV_threshold);
					createTrackbar("Smin", "HSV", &low_S, 255, mod_HSV_threshold);
					createTrackbar("Smax", "HSV", &high_S, 255, mod_HSV_threshold);
					createTrackbar("Vmin", "HSV", &low_V, 255, mod_HSV_threshold);
					createTrackbar("Vmax", "HSV", &high_V, 255, mod_HSV_threshold);
					mod_HSV_threshold(0,0);
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
            cout << "  Mouse X, Y: " << x << ", " << y ;
            cout << endl;
			colorsBGR = currentImage.at<Vec3b>(y,x);
			cout << "B: " << int(colorsBGR[0])<< " G: " << int(colorsBGR[1])<< " R: " << int(colorsBGR[2])<<endl;
			//Open new window with solid color of pixel at click
			imshow("Color", solidColor.setTo(Scalar(int(colorsBGR[0]), int(colorsBGR[1]), int(colorsBGR[2]))));
            /*  Draw a point */
            points.push_back(Point(x, y));
            break;
        case CV_EVENT_MOUSEMOVE:
            break;
        case CV_EVENT_LBUTTONUP:
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

  /// Display
  namedWindow("Histogram B", CV_WINDOW_AUTOSIZE );
  imshow("Histogram B", histImageB );
namedWindow("Histogram G", CV_WINDOW_AUTOSIZE );
  imshow("Histogram G", histImageG );
namedWindow("Histogram R", CV_WINDOW_AUTOSIZE );
  imshow("Histogram R", histImageR );
}
