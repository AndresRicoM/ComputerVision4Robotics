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
vector<Point> puntos;

// Frozen picture
bool frozen = false, closeP=false;

int trackValue = 0;
int low_H = 0, low_S = 0, low_V = 0;
int high_H = 255, high_S = 255, high_V = 255;


/* This is the callback that will only display mouse coordinates */
void mouseCoordinatesExampleCallback(int event, int x, int y, int flags, void* param);

/* Create images where captured and transformed frames are going to be stored */
Mat currentImage;

/* Create images where captured and transformed frames are going to be stored */
Mat auxImage;
Mat hsv_thres;
Mat GUI;
Mat output;
// Create image template for coloring
Mat solidColor(50, 50, CV_8UC3);


void mod_HSV_threshold(int,void*){
	cvtColor(currentImage, auxImage, cv::COLOR_BGR2HSV);
	inRange(auxImage, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), hsv_thres);
	imshow("HSV", currentImage);
	imshow("HSVTHres", hsv_thres);
	currentImage.copyTo(output, hsv_thres);
	imshow("HSV output", output);
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
			// copy contents from camera to graphic user interface image
			currentImage.copyTo(GUI);

			// paint range selection rectangle on screen (GUI)
			if (points.size() > 1) {
				rectangle(GUI, (Point)points[0], (Point)points[1], Scalar( 0, 255, 0 ), 1, 8, 0);
			}
			/* Show GUI on main window */
			imshow("Image", GUI);

			 if(!frozen){
        		if(!(getWindowProperty("HSV", WND_PROP_AUTOSIZE) == -1)){
          			mod_HSV_threshold(0,0);
        		}}


			// Keypress actions
			switch(waitKey(3)){
				case 'f':
					/* If 'f' is pressed, freeze image */
					frozen = not frozen;
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








// Store BGR values of a point in image
Vec3b channels;
Vec3i minRange;
Vec3i maxRange;

// Function to autoset ranges for thresholds in color spaces
void getParameterRange(){
	if(!(getWindowProperty("HSV", WND_PROP_AUTOSIZE) == -1)){
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
            }

            /*  Store a point */
			points.push_back(Point(x, y));

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



//aplicar diferentes mascaras a diferentes tonalidades de color
//1 rango con x y y
// otro rango con otro x y x
//las dos mascaras en un OR


//aplicar filtros a las mascaras
//en base a las mascaras sacar los contornos de cada una

//con los momentos, sacar el pixel del centro y llamar a segmentacion
//con los mismos momentos, sacar los momentos de HU