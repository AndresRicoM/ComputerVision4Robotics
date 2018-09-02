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
bool frozen = false;

int trackValue = 0;

/* Create images where captured and transformed frames are going to be stored */
	Mat auxImage;

/* This is the callback that will only display mouse coordinates */
void mouseCoordinatesExampleCallback(int event, int x, int y, int flags, void* param);

void modThreshold(int,void*)
{
	Mat tempImage;
	threshold( auxImage, tempImage, trackValue, 255, 0);
	imshow("B&W", tempImage);
}

int main(int argc, char *argv[])
{
	/* First, open camera device */
	VideoCapture camera;
    camera.open(0);

	/* Create images where captured and transformed frames are going to be stored */
	Mat currentImage;

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

			/* If 'f' is pressed, freeze image */
			if (waitKey(3) == 'f'){
				frozen = not frozen;
			}
			/* If 'b' is pressed, b&w image */
			if (waitKey(3) == 'b'){
				cvtColor(currentImage, auxImage, cv::COLOR_RGB2GRAY);
				imshow("B&W", auxImage);
				createTrackbar("Umbral", "B&W", &trackValue, 255, modThreshold);

			}
			if (waitKey(3) == 'h'){
				cvtColor(currentImage, auxImage, cv::COLOR_RGB2GRAY);
				imshow("HSV", auxImage);
				createTrackbar("Hmax", "HSV", &trackValue, 255, modThreshold);
				createTrackbar("Hmin", "HSV", &trackValue, 255, modThreshold);
				createTrackbar("Vmax", "HSV", &trackValue, 255, modThreshold);
				createTrackbar("Vmin", "HSV", &trackValue, 255, modThreshold);
				createTrackbar("Smax", "HSV", &trackValue, 255, modThreshold);
				createTrackbar("Smin", "HSV", &trackValue, 255, modThreshold);


			}

			/* If 'x' is pressed, exit program */
			if (waitKey(3) == 'x')
				break;
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
            /*  Draw a point */
            points.push_back(Point(x, y));
            break;
        case CV_EVENT_MOUSEMOVE:
            break;
        case CV_EVENT_LBUTTONUP:
            break;
    }
}
