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

// Arreglo para mapeado de identificador con color para segmentacion
Vec3i colors[5] ={
  Vec3i(255, 244, 30), // Light Blue
  Vec3i(51, 255, 255), // Yellow
  Vec3i(47, 255, 173), // Green
  Vec3i(0, 140, 255), // Orange
  Vec3i(0, 69, 255) // Red
};

int coloringIndex = 0;

/* Funcion para ejecutar gota de aceite
 P = punto a expandir
 K = matriz de regiones
 F = imagen de entrada (binarizada)
*/
void gotaDeAceite(Point P, Mat K, Mat F){
  //List for Gota de Aceite
  //list<Point> Fo;
  vector<Point> Fo; //Lista de puntos por procesar
  Point vecinos[4] = {Point(0,1),Point(0,-1),Point(1,0),Point(-1,0)}; //Coordenadas vecinos
  Point Pa, Pe; // Punto adjacente, punto evaluado
  Fo.push_back(P);

  while (!Fo.empty()){
    Pe = Fo.back();
    Fo.pop_back();
    for(int i = 0; i < 4 ; i++){
      Pa = Pe + vecinos[i];
      Vec3b rgbChannelsA = K.at<Vec3b>(Pa.y,Pa.x); // Valores rgb en Pa de K (Matriz de regiones)
      Vec3b rgbChannelsE = F.at<Vec3b>(Pa.y,Pa.x); // Valores rgb en Pa de F (Imagen de entrada)

      //  if (Color(Pa)=0 and f(Pa)=1)
      if(int(rgbChannelsA[0]) == 0 && int(rgbChannelsA[1]) == 0 && int(rgbChannelsA[2]) == 0 &&
        int(rgbChannelsE[0]) == 255 && int(rgbChannelsE[1]) == 255 && int(rgbChannelsE[2]) == 255){
          K.at<Vec3b>(Pa.y,Pa.x) = colors[coloringIndex];
          Fo.push_back(Pa);
      }
      //
    }
  }
}




// Here we will store points
vector<Point> points;
vector<Point> puntos;

// Frozen picture
bool frozen = false, closeP=false;

int trackValue = 0;
int low_H = 0, low_S = 0, low_V = 0;
int high_H = 255, high_S = 255, high_V = 255;

int low_H2 = 0, low_S2 = 0, low_V2 = 0;
int high_H2 = 255, high_S2 = 255, high_V2 = 255;


/* This is the callback that will only display mouse coordinates */
void mouseCoordinatesExampleCallback(int event, int x, int y, int flags, void* param);

/* Create images where captured and transformed frames are going to be stored */
Mat currentImage;

/* Create images where captured and transformed frames are going to be stored */
Mat auxImage;
Mat hsv_thres;
Mat hsv2_thres;
Mat GUI;
Mat output;
Mat output2;

//Merging of both masks
Mat res;
// Create image template for coloring
Mat solidColor(50, 50, CV_8UC3);

//contador de veces que se seleccionan colores
int cont= 0;

void mod_HSV_threshold(int,void*){
	cvtColor(currentImage, auxImage, cv::COLOR_BGR2HSV);
	

	inRange(auxImage, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), hsv_thres);
	inRange(auxImage, Scalar(low_H2, low_S2, low_V2), Scalar(high_H2, high_S2, high_V2), hsv2_thres);


	///////////*********DIlation*********************** For filtering masks
    //https://docs.opencv.org/2.4/doc/tutorials/imgproc/opening_closing_hats/opening_closing_hats.html
    // Since MORPH_X : 2,3,4,5 and 6
  	int operation = 3;
  	int morph_size = 5;

  	Mat element = getStructuringElement( 0, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
  	/// Apply the specified morphology operation
  	blur( hsv_thres, hsv_thres, Size(5,5) );
  	blur( hsv2_thres, hsv2_thres, Size(5,5) );
  	morphologyEx( hsv_thres, hsv_thres, operation, element );
  	morphologyEx( hsv2_thres, hsv2_thres, operation, element );

	//Combine masks
	bitwise_or(hsv_thres,hsv2_thres,res);

	//////////////////**********GET CONTOURS ***********////////////////////
  	//https://docs.opencv.org/2.4/doc/tutorials/imgproc/shapedescriptors/moments/moments.html
  	int thresh = 100;
	int max_thresh = 255;
	RNG rng(12345);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	/// Find contours
  	findContours( res, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  	Mat drawing = Mat::zeros( res.size(), CV_8UC3 );


  	//******************/////// get the mass center
  	// Create empty mat
  	//Mat regiones = Mat::zeros(hsv_thres.rows, hsv_thres.cols, CV_8UC3);
  	//Moments m = moments(hsv_thres, true);
    //Point center(m.m10/m.m00, m.m01/m.m00);
    //gotaDeAceite(center,hsv_thres, regiones);

    //circle(regiones, center, 5, Scalar(128,0,0), -1);

    //imshow("Regiones", regiones);

    //*****************////////////


  	int largest_contour_index=0;
  	int largest_area=0;
  	for( size_t i = 0; i< contours.size(); i++ ){

		//Find the area of contour
        double a=contourArea( contours[i],false);
        //if a> largest_area
        if(a>300){
            
        	Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
            drawContours( drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );

            //largest_area=a;
            // Store the index of largest contour
            //largest_contour_index=i;               
        }
        //else{
        //	if (a>secondlargest_area){
        //		secondlargest_area = a;
        //		secondlargest_contour_index=i;
        //	}

    }
	//Scalar color2( 255,255,255);  // color of the contour in the
    //Draw the contour and rectangle
    //drawContours( drawing2, contours,largest_contour_index, color2, 2,8,hierarchy);

    //	Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
    //	drawContours( drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );
    

    namedWindow( "Contours", WINDOW_AUTOSIZE );
	imshow( "Contours", drawing );
	
	imshow("HSV12 Res", res);
	currentImage.copyTo(output, hsv_thres);

	imshow("HSV", output);
	output.release();
	currentImage.copyTo(output2, hsv2_thres);
	imshow("HSV2", output2);
	output2.release();
	
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
					imshow("HSV2", currentImage);
					createTrackbar("Hmin", "HSV2", &low_H2, 255, mod_HSV_threshold);
					createTrackbar("Hmax", "HSV2", &high_H2, 255, mod_HSV_threshold);
					createTrackbar("Smin", "HSV2", &low_S2, 255, mod_HSV_threshold);
					createTrackbar("Smax", "HSV2", &high_S2, 255, mod_HSV_threshold);
					createTrackbar("Vmin", "HSV2", &low_V2, 255, mod_HSV_threshold);
					createTrackbar("Vmax", "HSV2", &high_V2, 255, mod_HSV_threshold);
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
		if (cont==0){
			setTrackbarPos("Hmin", "HSV2", minRange[0]-1);
			setTrackbarPos("Smin", "HSV2", minRange[1]-30);
			setTrackbarPos("Vmin", "HSV2", minRange[2]-25);
			setTrackbarPos("Hmax", "HSV2", maxRange[0]+1);
			setTrackbarPos("Smax", "HSV2", maxRange[1]+30);
			setTrackbarPos("Vmax", "HSV2", maxRange[2]+25);
		}else{
		setTrackbarPos("Hmin", "HSV", minRange[0]-1);
		setTrackbarPos("Smin", "HSV", minRange[1]-30);
		setTrackbarPos("Vmin", "HSV", minRange[2]-25);
		setTrackbarPos("Hmax", "HSV", maxRange[0]+1);
		setTrackbarPos("Smax", "HSV", maxRange[1]+30);
		setTrackbarPos("Vmax", "HSV", maxRange[2]+25);
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
        	cont+=1;
        	if (cont>1){
        		cont=0;
        	}
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