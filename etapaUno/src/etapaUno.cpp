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
	
	// Create image template for coloring
	Mat solidColor(50, 50, CV_8UC3);

	/* Create images where captured and transformed frames are going to be stored */
	Mat currentImage;

// Store BGR values of a point in image
Vec3b colorsBGR;

/* This is the callback that will only display mouse coordinates */
void mouseCoordinatesExampleCallback(int event, int x, int y, int flags, void* param);





void modThreshold(int,void*)
{
	Mat tempImage;
	threshold( auxImage, tempImage, trackValue, 255, 0);
	imshow("B&W", tempImage);
}

void mod_HSV_threshold(int,void*)
{
	inRange(auxImage, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), hsv_thres);
	imshow("HSV Threshold", hsv_thres);
}

void mod_BGR_threshold(int,void*)
{
	inRange(currentImage, Scalar(low_B, low_G, low_R), Scalar(high_B, high_G, high_R), bgr_thres);
	imshow("RGB Threshold", bgr_thres);
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
					imshow("RGB", currentImage);
					inRange(currentImage, Scalar(low_B, low_G, low_R), Scalar(high_B, high_G, high_R), bgr_thres);
					imshow("RGB Threshold", bgr_thres);
					createTrackbar("Bmin", "RGB", &low_B, 255, mod_BGR_threshold);
					createTrackbar("Bmax", "RGB", &high_B, 255, mod_BGR_threshold);
					createTrackbar("Gmin", "RGB", &low_G, 255, mod_BGR_threshold);
					createTrackbar("Gmax", "RGB", &high_G, 255, mod_BGR_threshold);
					createTrackbar("Rmin", "RGB", &low_R, 255, mod_BGR_threshold);
					createTrackbar("Rmax", "RGB", &high_R, 255, mod_BGR_threshold);
					break;
				case 'h':
					/* If 'h' is pressed, hsv image */
					cvtColor(currentImage, auxImage, cv::COLOR_BGR2HSV);
					imshow("HSV", auxImage);
					inRange(auxImage, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), hsv_thres);
					imshow("HSV Threshold", hsv_thres);
					createTrackbar("Hmin", "HSV", &low_H, 255, mod_HSV_threshold);
					createTrackbar("Hmax", "HSV", &high_H, 255, mod_HSV_threshold);
					createTrackbar("Smin", "HSV", &low_S, 255, mod_HSV_threshold);
					createTrackbar("Smax", "HSV", &high_S, 255, mod_HSV_threshold);
					createTrackbar("Vmin", "HSV", &low_V, 255, mod_HSV_threshold);
					createTrackbar("Vmax", "HSV", &high_V, 255, mod_HSV_threshold);
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
