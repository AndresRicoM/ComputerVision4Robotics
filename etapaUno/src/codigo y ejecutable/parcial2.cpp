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
#include <fstream>

//#include <list>

#define CV_CAST_8U

using namespace cv;
using namespace std;

// Array to map segmentation id color
Vec3i colors[5] ={
  Vec3i(255, 244, 30), // Light Blue
  Vec3i(51, 255, 255), // Yellow
  Vec3i(47, 255, 173), // Green
  Vec3i(0, 140, 255), // Orange
  Vec3i(0, 69, 255) // Red
};

int coloringIndex = 0;

//global variables for bullsEye drawing
int activeQuadrant = 0;
int currentAngle = 20;


// Here we will store points
vector<Point> points;
vector<Point> histPoints;

vector<Point> segSeeds;
vector<Point> segSeedAux;


// Frozen picture
bool frozen = false, closeP=false;

int trackValue = 0;

int low_H = 0, low_S = 0, low_V = 0;
int high_H = 255, high_S = 255, high_V = 255;
int iClosing = 5,iClosing2 = 5, iOpening = 5;


/* Create images where captured and transformed frames are going to be stored */
	Mat auxImage;
	Mat hsv_thres;
  Mat auxImage3;
	Mat output;
	Mat GUI;

  Mat drawing = Mat::zeros( hsv_thres.size(), CV_8UC3 );


	// Create image template for coloring
	Mat solidColor(50, 50, CV_8UC3);

	/* Create images where captured and transformed frames are going to be stored */
	Mat currentImage;

// Store BGR values of a point in image
Vec3b channels;

Vec3i minRange;
Vec3i maxRange;

/* This is the callback that will only display mouse coordinates and update parameter range*/
void mouseOnMainImageCallback(int event, int x, int y, int flags, void* param);

void mouseHSVregionCallback(int event, int x, int y, int flags, void* param);

void mod_HSV_threshold(int,void*);
void getParameterRange();
void hsvHistogram();
void drawBullsEye();
void openAndCloseFilter(int,void*);
void updateSegContCen();
void gotaDeAceite(Point seed, Mat outColored, Mat inputMat);
void find_moments(Mat segmentMoment, ofstream &myfile);
void plotF1F2(double fi[2]);
Mat segmentar(Mat binaria);
Mat getContours(Mat regiones);
Point getMassCenter(Mat segmentos);

//Array to keep track of presence of figures in camera image
bool presentFigures[4];

enum knownFigures { BB8, Lightsaber, R2D2, Spaceship };

RNG rng(12345);
ofstream myfile;

/* --------------------------------------------------------------------------------------*
 * #####   MAIN  ######                                                 					       *
 * ------------------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
  bool histogramOn = false;
  bool bullsEyeOn = false;
	/* First, open camera device */
	VideoCapture camera;
    camera.open(0);


    /* Create main OpenCV window to attach callbacks */
    namedWindow("Image");
    setMouseCallback("Image", mouseOnMainImageCallback);

    myfile.open ("nothing.csv");
    myfile << "fi1,fi2,name\n";

    histPoints.push_back(Point(0,0));


    while (true)
    {
  		if (!frozen)
      {
  			/* Obtain a new frame from camera */
  			camera >> currentImage;
        // Inicializar regiones con zeros
        //regiones = Mat::zeros(currentImage.rows, currentImage.cols, CV_8UC3);

        // Si la imagen no esta congelada y la ventana de regiones esta abierta
        // segmentar las semillas y actualizar la imagen
        if(!(getWindowProperty("Segmentos", WND_PROP_AUTOSIZE) == -1))
        {
          updateSegContCen();
        }
      }

  		if (currentImage.data)
  		{
  			// copy contents from camera to graphic user interface image
  			currentImage.copyTo(GUI);

  			// paint range selection rectangle on screen (GUI)
  			if (points.size() > 1)
        {
  				rectangle(GUI, (Point)points[0], (Point)points[1], Scalar( 0, 255, 0 ), 1, 8, 0);
  			}


  			/* Show GUI on main window */
  			imshow("Image", GUI);
        moveWindow("Image", 0, 0);
        if(!frozen)
        {
          if(!(getWindowProperty("HSV", WND_PROP_AUTOSIZE) == -1))
          {
            mod_HSV_threshold(0,0);
            if(histogramOn)
            {
              hsvHistogram();
            }
          }
          if(!(getWindowProperty("bullsEye", WND_PROP_AUTOSIZE) == -1))
          {
            drawBullsEye();
          }
        }
        else
        {
          if(!(getWindowProperty("HSV", WND_PROP_AUTOSIZE) == -1) && histogramOn)
          {
            hsvHistogram();
          }
        }


  			// Keypress actions
  			switch(waitKey(3))
        {
  				case 'f':
  					/* If 'f' is pressed, freeze image */
  					frozen = not frozen;
  					break;
  				case 'h':
  					/* If 'h' is pressed, hsv tools */
  					imshow("HSV", currentImage);
            moveWindow("HSV", 650, 0);
            setMouseCallback("HSV", mouseHSVregionCallback);
  					createTrackbar("Hmin", "HSV", &low_H, 255, mod_HSV_threshold);
  					createTrackbar("Hmax", "HSV", &high_H, 255, mod_HSV_threshold);
  					createTrackbar("Smin", "HSV", &low_S, 255, mod_HSV_threshold);
  					createTrackbar("Smax", "HSV", &high_S, 255, mod_HSV_threshold);
  					createTrackbar("Vmin", "HSV", &low_V, 255, mod_HSV_threshold);
  					createTrackbar("Vmax", "HSV", &high_V, 255, mod_HSV_threshold);
            createTrackbar("Closing", "HSV", &iClosing, 10, openAndCloseFilter);
            createTrackbar("Opening", "HSV", &iOpening, 10, openAndCloseFilter);
            createTrackbar("Closing2", "HSV", &iClosing2, 10, openAndCloseFilter);
  					mod_HSV_threshold(0,0);
  					break;
          case 'c':
              // If 'c' is pressed clean segSeeds vector
              segSeeds.clear();

              imshow("Segmentos", Mat::zeros(hsv_thres.rows, hsv_thres.cols, CV_8UC3));

              break;
          case 'i':
              // Toggle histograms
              if(!(getWindowProperty("Histogram H", WND_PROP_AUTOSIZE) == -1)){
                destroyWindow("Histogram H");
              }
              if(!(getWindowProperty("Histogram S", WND_PROP_AUTOSIZE) == -1)){
                destroyWindow("Histogram S");
              }
              if(!(getWindowProperty("Histogram V", WND_PROP_AUTOSIZE) == -1)){
                destroyWindow("Histogram V");
              }
              histogramOn = !histogramOn;
              break;
          case 'b':
              // Toggle bullsEye
              if(!(getWindowProperty("bullsEye", WND_PROP_AUTOSIZE) == -1))
              {
                destroyWindow("bullsEye");
              }
              else
              {
                drawBullsEye();
              }
              bullsEyeOn = !bullsEyeOn;
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

/* --------------------------------------------------------------------------------------*
 * ----   Callback for mouse clicks on main Window          ----          	  		       *
 * ------------------------------------------------------------------------------------- */
void mouseOnMainImageCallback(int event, int x, int y, int flags, void* param)
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

/* --------------------------------------------------------------------------------------*
 * ----   Callback for mouse click on HSV threshold window   ----      		               *
 *           -Adds a segmentation Seed                                                   *
 * ------------------------------------------------------------------------------------- */
void mouseHSVregionCallback(int event, int x, int y, int flags, void* param)
{
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:

            segSeeds.push_back(Point(x,y));
            //Push_front
            //segSeeds.emplace ( segSeeds.begin()+segSeeds.size(), Point(x,y) );
            updateSegContCen();

            break;
    }
}

/* --------------------------------------------------------------------------------------*
 * ----   Function to update Segmentation, Contours, and Centroids   ----      		       *
 * ------------------------------------------------------------------------------------- */
void updateSegContCen()
{
  // Get segments
  Mat segmentos = Mat::zeros( hsv_thres.size(), CV_8UC3 );
  segmentos = segmentar(hsv_thres);

  Mat segmentosGris = Mat::zeros( hsv_thres.size(), CV_8UC1 );
  cvtColor(segmentos, segmentosGris, cv::COLOR_BGR2GRAY);

  // Get contours
  Mat contoursMat = Mat::zeros( segmentos.size(), CV_8UC3 );
  contoursMat = getContours(segmentos);
  //Pintar centros
  for(Point i : segSeeds){
    circle(contoursMat, i, 3, 255, -1);
  }
  // Print segments
  imshow("Segmentos", segmentos);
  // Print contours
  imshow("Contornos", contoursMat);
  ~Mat(segmentos);
  ~Mat(contoursMat);
}

// Segmentar Mat(CV_8UC1) binarizada y regresar Mat(CV_8UC3) coloreada por segmentos
Mat segmentar(Mat binaria){
  Mat segmentosOut;
  Mat auxSeg;
  // Hay que corregir que Mat binaria viene con un solo canal, se debe pasar a 3 canales
  vector<Mat> nueva;
  nueva.push_back(binaria);
  nueva.push_back(binaria);
  nueva.push_back(binaria);
  merge(nueva, auxSeg);

  // Create empty mat
  segmentosOut = Mat::zeros(auxSeg.rows, auxSeg.cols, CV_8UC3);

  segSeedAux.clear();
  // Iterar sobre las semillas y segmentar
  for(Point i : segSeeds){
    // Obtener region
    gotaDeAceite(i, segmentosOut, auxSeg);
    coloringIndex++;
  }

  segSeeds = segSeedAux;
  coloringIndex = 0;

  return segmentosOut;
}

/* --------------------------------------------------------------------------------------*
 * ----   Callback for manipulation of HSV threshold trackbars   ----                    *
 * ------------------------------------------------------------------------------------- */
void mod_HSV_threshold(int,void*)
{
	cvtColor(currentImage, auxImage, cv::COLOR_BGR2HSV);
	inRange(auxImage, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), hsv_thres);
	currentImage.copyTo(output, hsv_thres);

  //Apply dilation filter on hsv_thres
  openAndCloseFilter(0,0);


  // Graph seeds for segmentation in HSV window (HSV mask window)
  if (!segSeeds.empty()){
    vector<Mat> nueva;
    nueva.push_back(hsv_thres);
    nueva.push_back(hsv_thres);
    nueva.push_back(hsv_thres);

    merge(nueva, auxImage3);
    for(Point i : segSeeds){
      circle(auxImage3, i, 3, Scalar(255, 244, 30), -1);
    }
    imshow("HSV", auxImage3);
  }else{
    imshow("HSV", hsv_thres);
  }

	imshow("HSV Threshold", output);
  moveWindow("HSV Threshold", 1300, 0);
	output.release();
}


/* --------------------------------------------------------------------------------------*
 * ----   Function to autoset ranges for thresholds in color spaces   ----               *
 * ------------------------------------------------------------------------------------- */
void getParameterRange()
{
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

/* --------------------------------------------------------------------------------------*
 * ----   Function to obtain and graph histograms for HSV   ----      	        	       *
 * ------------------------------------------------------------------------------------- */
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

/* --------------------------------------------------------------------------------------*
 * ----   Function to draw BullsEye with active quadrant and angle line   ----      	   *
 * ------------------------------------------------------------------------------------- */
void drawBullsEye()
{
  int cuadrante = 0;
  //Assign global variable for current angle
  int angle = currentAngle;
  //Get corresponding quadrant from present figures
  if (presentFigures[Lightsaber])
  {
    if(presentFigures[BB8])
    {
      cuadrante = 1;
    }
    else if(presentFigures[R2D2])
    {
      cuadrante = 2;
    }
  }else if(presentFigures[Spaceship])
  {
    if(presentFigures[BB8])
    {
      cuadrante = 4;
    }
    else if(presentFigures[R2D2])
    {
      cuadrante = 3;
    }
  }
  double pi = 3.1415926535897;
  int height=500, width=500;
  int cRadius=200;
  int c1=1, c2=1, c3=1, c4=1;
  // create mat object to draw on
  Mat eye(height, width, CV_8UC3);
  //paint it white
  eye.setTo(Scalar(255,255,255));

  //Fill respective quadrant
  switch (cuadrante){
    case 1: c1 = -1; break;
    case 2: c2 = -1; break;
    case 3: c3 = -1; break;
    case 4: c4 = -1; break;
    default: break;
  }

  //draw circle quarters
  ellipse(eye, Point(width/2, height/2), Size(cRadius, cRadius), 0, 0, 90, Scalar( 0, 0, 255 ), c4, 8, 0);
  ellipse(eye, Point(width/2, height/2), Size(cRadius, cRadius), 0, 90, 180, Scalar( 0, 0, 255 ), c3, 8, 0);
  ellipse(eye, Point(width/2, height/2), Size(cRadius, cRadius), 0, 180, 270, Scalar( 0, 0, 255 ), c2, 8, 0);
  ellipse(eye, Point(width/2, height/2), Size(cRadius, cRadius), 0, 270, 360, Scalar( 0, 0, 255 ), c1, 8, 0);

  //draw red axis
  line(eye, Point(0, height/2), Point(width, height/2), Scalar( 0, 0, 255 ), 1, 8, 0);
  line(eye, Point(width/2, 0), Point(width/2, height), Scalar( 0, 0, 255 ), 1, 8, 0);

  //Draw angle line
  int angleX = cRadius * cos((-angle*pi)/180);
  int angleY = cRadius * sin((-angle*pi)/180);
  line(eye, Point(width/2+angleX, height/2+angleY), Point(width/2-angleX, height/2-angleY), Scalar( 255, 0, 0), 3, 8, 0);

  imshow("bullsEye", eye);
  moveWindow("bullsEye", 0, 600);
}


/* --------------------------------------------------------------------------------------*
 * ----   Function to apply opening and closing filter to Mat object   ----         	   *
 * ------------------------------------------------------------------------------------- */
void openAndCloseFilter(int,void*)
{
  int operation_Closing = 3; // Closing morphologyEx
  int operation_Opening = 2; // Opening morphologyEx


  Mat element1 = getStructuringElement( 0, Size( 2*iClosing + 1, 2*iClosing+1 ), Point( iClosing, iClosing ) );
  Mat element2 = getStructuringElement( 0, Size( 2*iOpening + 1, 2*iOpening+1 ), Point( iOpening, iOpening ) );
  Mat element3 = getStructuringElement( 0, Size( 2*iClosing2 + 1, 2*iClosing2+1 ), Point( iClosing2, iClosing2 ) );
  /// Apply the specified morphology operation
  //blur( hsv_thres, hsv_thres, Size(5,5) );

  morphologyEx( hsv_thres, hsv_thres, operation_Closing, element1 );
  morphologyEx( hsv_thres, hsv_thres, operation_Opening, element2 );
  morphologyEx( hsv_thres, hsv_thres, operation_Closing, element3 );
}

/* --------------------------------------------------------------------------------------*
 * ----   Function to obtain contours from Mat object   ----         	                   *
 * ------------------------------------------------------------------------------------- */
Mat getContours(Mat regiones)
{

  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  Mat contoursMat = Mat::zeros( regiones.size(), CV_8UC3 );
  //Convert
  cvtColor(regiones, contoursMat, cv::COLOR_BGR2GRAY);
  /// Find contours
  findContours( contoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  contoursMat = Mat::zeros( regiones.size(), CV_8UC1);

  for( size_t i = 0; i< contours.size(); i++ )
  {
    //Find the area of contour
    double a=contourArea( contours[i],false);
    //if a> largest_area
    if(a>300)
    {
      Scalar color = Scalar(255,255,255); //White
      drawContours( contoursMat, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );
    }
  }
  return contoursMat;
}

/* --------------------------------------------------------------------------------------*
 * ----   Function to obtain centroid of binary mat object   ----         	             *
 *            -returns a Point                                                           *
 * ------------------------------------------------------------------------------------- */
Point getMassCenter(Mat segmentos){
  // Create empty mat
  Moments m = moments(segmentos, true);
  Point center(m.m10/m.m00, m.m01/m.m00);

  return center;
}

/* --------------------------------------------------------------------------------------*
 * ----   Function to apply "Gota de Aceite" segmentation on Seed (Point P) ----         *
 *            -P = segmentation seed ; outColored = colored output mat ;                 *
 *            -inputMat = input binary Mat                                               *
 *            -calls get mass center to get centroid of each segment                     *                                       *
 * ------------------------------------------------------------------------------------- */
void gotaDeAceite(Point seed, Mat outColored, Mat inputMat){
  Mat segmentToMomento = Mat::zeros( outColored.rows, outColored.cols, CV_8UC1 );

  //List for Gota de Aceite
  //list<Point> Fo;
  vector<Point> Fo; //Lista de puntos por procesar
  Point vecinos[4] = {Point(0,1),Point(0,-1),Point(1,0),Point(-1,0)}; //Coordenadas vecinos
  Point Pa, Pe; // Punto adjacente, punto evaluado
  Fo.push_back(seed);

  while (!Fo.empty()){
    Pe = Fo.back();
    Fo.pop_back();
    for(int i = 0; i < 4 ; i++){
      Pa = Pe + vecinos[i];
      Vec3b rgbChannelsA = outColored.at<Vec3b>(Pa.y,Pa.x); // Valores rgb en Pa de outColored (Matriz de segmentos)
      Vec3b rgbChannelsE = inputMat.at<Vec3b>(Pa.y,Pa.x); // Valores rgb en Pa de inputMat (Imagen de entrada)

      //  if (Color(Pa)=0 and f(Pa)=1)
      if(int(rgbChannelsA[0]) == 0 && int(rgbChannelsA[1]) == 0 && int(rgbChannelsA[2]) == 0 &&
        int(rgbChannelsE[0]) == 255 && int(rgbChannelsE[1]) == 255 && int(rgbChannelsE[2]) == 255){
          //Segment for colorfull mat feedback
          outColored.at<Vec3b>(Pa.y,Pa.x) = colors[coloringIndex];
          //Segment to get individual momentos
          segmentToMomento.at<uchar>(Pa.y,Pa.x) = 255;
          Fo.push_back(Pa);
      }
      //
    }
  }

  // Swap original segSeed location with center of mass of segment
  Point center = getMassCenter(segmentToMomento);
  if((center.x>0)&&(center.x<segmentToMomento.cols))
  {
    segSeedAux.push_back(center);
  }
  find_moments(segmentToMomento, myfile);
}

int cuentame = 0;
/* --------------------------------------------------------------------------------------*
 * Function to calculate Hu Moments 									 	                                 *
 * ------------------------------------------------------------------------------------- */
void find_moments(Mat segmentMoment, ofstream &myfile)
{
		Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// Detect edges using canny
	//Canny(bin, canny_output, 50, 150, 3 );
  Canny(segmentMoment, canny_output, 50, 150, 3 );


	// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) ); //CV_RETR_TREE

	int largest_area = 0;
	int largest_contour_index = 0;

	if (contours.size() > 0){

	for( int i = 0; i< contours.size(); i++ )
  {
    //Find the area of contour
    double a=contourArea( contours[i],false);
    //if a> largest_area
    if(a>largest_area)
    {
      //Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
      //drawContours( drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );

      largest_area=a;
      // Store the index of largest contour
      largest_contour_index=i;
    }
  }


	Moments mu;                      // Moments
	double huMoments[3];	                  // Hu Moments
	double nMoments[3];					  // N Moments
	double gamma[3];
	gamma[0] = ((2+0)/2) + 1; 								  // Gamma 20
	gamma[1] = ((0+2)/2) + 1; 								  // Gamma 02
	gamma[2] = ((1+1)/2) + 1; 								  // Gamma 11
	double fi[2];							  // Fi Values
	Point2f centroid; 			  // Centroids

	double angle_rad, angle_g;								  // Orientation with Hu moments

	Point2f P2, P1;


  // Get Moments
  mu = moments(contours[largest_contour_index], false);
  // Get Hu Moments
  huMoments[0] = mu.m20 - ((mu.m10/mu.m00)*mu.m10); //Mu20
  huMoments[1] = mu.m02 - ((mu.m01/mu.m00)*mu.m01); //Mu02
  huMoments[2] = mu.m11 - ((mu.m01/mu.m00)*mu.m10); //Mu11
  huMoments[3] = mu.m00;                            //Mu00

  // Get N Moments
  nMoments[0] = huMoments[0] / (pow(mu.m00, gamma[0])); //n20
  nMoments[1] = huMoments[1] / (pow(mu.m00, gamma[1])); //n02
  nMoments[2] = huMoments[2] / (pow(mu.m00, gamma[2])); //n11

  // Get Fi values
  fi[0] = nMoments[0] + nMoments[1]; //Fi1
  fi[1] = pow((nMoments[0] - nMoments[1]), 2) + 4*pow(nMoments[2], 2); //Fi2

  // Orientation
  angle_rad = 0.5*atan2( (2*huMoments[2])  , (huMoments[0]- huMoments[1]) );
  angle_g = angle_rad * (180.00/3.1416);


  cout << "angulo rad: " << angle_rad << endl;
  cout << "angulo grado: " << angle_g << endl;

  printf("Fi1: %f \n", fi[0]);
  printf("Fi2: %f \n", fi[1]);

  myfile << fi[0] << "," << fi[1] << endl;


  // ***************************   TRAINING DATA *********************************
  double fi1_bb8 = 0.179271142;
  double fi2_bb8 = 0.004877734;
  double desv1_bb8 = 0.011026558;
  double desv2_bb8 = 0.005050658;

  double ddbb8 = pow((fi[0] - fi1_bb8),2) + pow((fi[1]-fi2_bb8),2);

  double fi1_ea = 0.48499072;
  double fi2_ea = 0.19502295;
  double desv1_ea = 0.01722625;
  double desv2_ea = 0.01254551;

  double ddea = pow((fi[0] - fi1_ea),2) + pow((fi[1]-fi2_ea),2);

  double fi1_r2 = 0.197311215;
  double fi2_r2 = 0.013102077;
  double desv1_r2 = 0.002949608;
  double desv2_r2 = 0.001129111;

  double ddr2 = pow((fi[0] - fi1_r2),2) + pow((fi[1]-fi2_r2),2);

  double fi1_nave = 0.318991118;
  double fi2_nave = 0.073201688;
  double desv1_nave = 0.01409311;
  double desv2_nave = 0.008670059;

  double ddnave = pow((fi[0] - fi1_nave),2) + pow((fi[1]-fi2_nave),2);

  // **********************   END OF TRAINING DATA *******************************

  double dmin = min(ddea,ddbb8);
  dmin = min(dmin, ddr2);
  dmin = min(dmin, ddnave);



  //Reset angle and figure presence for bullsEye drawing
  currentAngle = 0;
  if (cuentame>1){
  presentFigures[BB8] = false;
  presentFigures[Lightsaber] = false;
  presentFigures[R2D2] = false;
  presentFigures[Spaceship] = false;
  cuentame=0;
}
cuentame+=1;

  if (dmin == ddbb8)
  {
    if  ( ( ( fi1_bb8- desv1_bb8 ) < fi[0] ) &&  ( fi[0]  < (fi1_bb8 + desv1_bb8 )  )  )
    {
      if ( ( (fi2_bb8 - desv2_bb8) < fi[1] ) && (fi[1]  < (fi2_bb8 + desv2_bb8 ) ) )
      {
        cout << "bb8" << endl;
        presentFigures[BB8] = true; // BB8 present on screen
      }
    }
  }
  else if (dmin == ddea)
  {
    if (( (fi1_ea- 5*desv1_ea) < fi[0]) &&     (fi[0]< (fi1_ea + 5*desv1_ea) ) )
    {
      if (( (fi2_ea- 5*desv2_ea) < fi[1]) &&  (fi[1]< (fi2_ea + 5*desv2_ea) ) )
      {
        cout << "espadux" << endl;
        // Update angle for bullsEye drawing
        currentAngle = angle_g;
        presentFigures[Lightsaber] = true; // Lightsaber present on screen
      }
    }
  }
  else if (dmin == ddr2)
  {
    if (( (fi1_r2- 5*desv1_r2) < fi[0]) && (fi[0]< (fi1_r2 + 5*desv1_r2) ) )
    {
      if (( (fi2_r2- 5*desv2_r2) < fi[1]) && (fi[1]< (fi2_r2 + 5*desv2_r2) ) )
        {
          cout << "r2" << endl;
          presentFigures[R2D2] = true; // R2D2 present on screen
        }
    }
  }
  else if (dmin == ddnave)
  {
    if (( (fi1_nave- 5*desv1_nave) < fi[0]) && (fi[0]< (fi1_nave + 5*desv1_nave) ) )
    {
      if (( (fi2_nave- 5*desv2_nave) < fi[1]) && (fi[1]< (fi2_nave + 5*desv2_nave) ) )
        {
          cout << "nave" << endl;
          // Update angle for bullsEye drawing
          currentAngle = angle_g;
          presentFigures[Spaceship] = true; // Spaceship present on screen
        }
    }
  }
  else
  {
    cout <<"No se encuentra ninguna figura" <<endl;
  }
  plotF1F2(fi);
}

}
/* --------------------------------------------------------------------------------------*
 * Function to plot F1 and F2 from Hu moments analisis 									 	               *
 * ------------------------------------------------------------------------------------- */
void plotF1F2(double fi[2])
{
  Scalar color_point = Scalar(51, 255, 255); // Yellow
  Scalar color_line = Scalar(47, 255, 173); // Green
  Mat plano = Mat::zeros( hsv_thres.size(), CV_8UC3 );
  // Draw Fi values 320x240
  Point xi, xf, yi, yf, textf1, textf2;
  xi.x = 0; xi.y = 0;
  xf.x = 320; xf.y = 0;
  yi.x = 0; yi.y = 0;
  yf.x = 0; yf.y = 240;
  textf1.x = 280; textf1.y = 35;
  textf2.x = 10; textf2.y = 235;
  line(plano, xi, xf, color_line, 10, 8, 0);
  line(plano, yi, yf, color_line, 10, 8, 0);
  putText(plano, "Fi1", textf1, FONT_HERSHEY_SIMPLEX, 1, color_point, 1, 8, 0);
  putText(plano, "Fi2", textf2, FONT_HERSHEY_SIMPLEX, 1, color_point, 1, 8, 0);
  // Plotear fi values
  Point P_fi;
  P_fi.x = (fi[0] * 320); //Fi1
  P_fi.y = (fi[1] * 240); //Fi2
  circle(plano, P_fi, 0, color_point, 15, 8, 0);

  namedWindow("Plano", CV_WINDOW_AUTOSIZE);
  imshow("Plano", plano);
}
