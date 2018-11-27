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

#include <list>

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
int currentAngle = 0;


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
	//Mat output;
	Mat GUI;

  Mat drawing = Mat::zeros( hsv_thres.size(), CV_8UC3 );

  Mat parkingOriginal;


	// Create image template for coloring
	Mat solidColor(50, 50, CV_8UC3);

	/* Create images where captured and transformed frames are going to be stored */
	Mat currentImage;

  Mat parkingLimits_1channel;

// Store BGR values of a point in image
Vec3b channels;

Vec3i minRange;
Vec3i maxRange;

/* This is the callback that will only display mouse coordinates and update parameter range*/
void mouseOnMainImageCallback(int event, int x, int y, int flags, void* param);

void mouseHSVregionCallback(int event, int x, int y, int flags, void* param);

void mouseOnParkingImage(int event, int x, int y, int flags, void* param);

void mod_HSV_threshold(int,void*);
void getParameterRange();
void hsvHistogram();
void drawBullsEye();
void openAndCloseFilter(int,void*);
void updateSegContCen();
void gotaDeAceite(Point seed, Mat outColored, Mat inputMat);
void find_moments(Mat segmentMoment, ofstream &myfile);
void plotF1F2(double fi[2]);
void updateTrainingMoment(Mat segmentToMomento);
void calculateTrainedPhi();
void printParking();
Mat segmentar(Mat binaria);
Mat getContours(Mat regiones);
Point getMassCenter(Mat segmentos);
Mat getNavigationBase(int parkingTargetIndex);

//Array to keep track of presence of figures in camera image
bool presentFigures[4];

struct myPiStruct
{
  double phiG1;
  double phiG2;
};

struct trainedDataStruct
{
  double fi1;
  double fi2;
  double desv1;
  double desv2;
  vector<myPiStruct> fiTVector;
};

trainedDataStruct trainedData[4];

struct parkingSpacesStruct
{
  Point polyPoints[4];
  Point centroid;
};

parkingSpacesStruct parkingSpaces[10];

bool trainingActive = false;
int trainingId = 4;
myPiStruct fiTrainingPair;

enum knownFigures { BB8, Lightsaber, R2D2, Spaceship };

RNG rng(12345);
ofstream myfile;

Point puntoInicio;

/* --------------------------------------------------------------------------------------*
 * #####   MAIN  ######                                                 					       *
 * ------------------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
  // ***************************   TRAINING DATA *********************************
  trainedData[0].fi1 = 0.179271142;
  trainedData[0].fi2 = 0.004877734;
  trainedData[0].desv1 = 0.011026558;
  trainedData[0].desv2 = 0.005050658;


  trainedData[1].fi1 = 0.48499072;
  trainedData[1].fi2 = 0.19502295;
  trainedData[1].desv1 = 0.01722625;
  trainedData[1].desv2 = 0.01254551;

  trainedData[2].fi1 = 0.197311215;
  trainedData[2].fi2 = 0.013102077;
  trainedData[2].desv1 = 0.002949608;
  trainedData[2].desv2 = 0.001129111;


  trainedData[3].fi1 = 0.318991118;
  trainedData[3].fi2 = 0.073201688;
  trainedData[3].desv1 = 0.01409311;
  trainedData[3].desv2 = 0.008670059;

  // ***************************   TRAINING DATA *********************************

  //Load original parking image
  parkingOriginal = imread("/home/yesus/Documents/vision/nf1/vision/imagen.png", CV_LOAD_IMAGE_COLOR);
  // ***************************   HARDCODED MAP *********************************
  parkingLimits_1channel = Mat::zeros( parkingOriginal.rows, parkingOriginal.cols, CV_8UC1 );
  Point polyPoints[4];
  polyPoints[0] = Point(0,0);
  polyPoints[1] = Point(54,0);
  polyPoints[2] = Point(66,478);
  polyPoints[3] = Point(0,474);
  cv::fillConvexPoly(parkingLimits_1channel, polyPoints, 4, 255, CV_AA, 0);
  polyPoints[0] = Point(115,119);
  polyPoints[1] = Point(230,123);
  polyPoints[2] = Point(235,485);
  polyPoints[3] = Point(122,477);
  cv::fillConvexPoly(parkingLimits_1channel, polyPoints, 4, 255, CV_AA, 0);
  polyPoints[0] = Point(289,125);
  polyPoints[1] = Point(409,127);
  polyPoints[2] = Point(413,493);
  polyPoints[3] = Point(293,485);
  cv::fillConvexPoly(parkingLimits_1channel, polyPoints, 4, 255, CV_AA, 0);
  polyPoints[0] = Point(466,132);
  polyPoints[1] = Point(588,135);
  polyPoints[2] = Point(591,502);
  polyPoints[3] = Point(471,496);
  cv::fillConvexPoly(parkingLimits_1channel, polyPoints, 4, 255, CV_AA, 0);
  polyPoints[0] = Point(645,80);
  polyPoints[1] = Point(700,83);
  polyPoints[2] = Point(700,560);
  polyPoints[3] = Point(648,560);
  cv::fillConvexPoly(parkingLimits_1channel, polyPoints, 4, 255, CV_AA, 0);
  polyPoints[0] = Point(122,0);
  polyPoints[1] = Point(613,0);
  polyPoints[2] = Point(614,74);
  polyPoints[3] = Point(123,59);
  cv::fillConvexPoly(parkingLimits_1channel, polyPoints, 4, 255, CV_AA, 0);



  parkingSpaces[0].polyPoints[0] = Point(182,401);
  parkingSpaces[0].polyPoints[1] = Point(240,402);
  parkingSpaces[0].polyPoints[2] = Point(240,424);
  parkingSpaces[0].polyPoints[3] = Point(182,421);
  parkingSpaces[1].polyPoints[0] = Point(182,456);
  parkingSpaces[1].polyPoints[1] = Point(240,458);
  parkingSpaces[1].polyPoints[2] = Point(240,484);
  parkingSpaces[1].polyPoints[3] = Point(182,477);
  parkingSpaces[2].polyPoints[0] = Point(290,406);
  parkingSpaces[2].polyPoints[1] = Point(349,409);
  parkingSpaces[2].polyPoints[2] = Point(349,431);
  parkingSpaces[2].polyPoints[3] = Point(290,429);
  parkingSpaces[3].polyPoints[0] = Point(353,241);
  parkingSpaces[3].polyPoints[1] = Point(412,245);
  parkingSpaces[3].polyPoints[2] = Point(414,267);
  parkingSpaces[3].polyPoints[3] = Point(354,264);
  parkingSpaces[4].polyPoints[0] = Point(394,17);
  parkingSpaces[4].polyPoints[1] = Point(418,17);
  parkingSpaces[4].polyPoints[2] = Point(418,72);
  parkingSpaces[4].polyPoints[3] = Point(395,73);
  parkingSpaces[5].polyPoints[0] = Point(449,16);
  parkingSpaces[5].polyPoints[1] = Point(473,16);
  parkingSpaces[5].polyPoints[2] = Point(473,74);
  parkingSpaces[5].polyPoints[3] = Point(451,75);
  parkingSpaces[6].polyPoints[0] = Point(463,275);
  parkingSpaces[6].polyPoints[1] = Point(524,277);
  parkingSpaces[6].polyPoints[2] = Point(524,299);
  parkingSpaces[6].polyPoints[3] = Point(462,300);
  parkingSpaces[7].polyPoints[0] = Point(469,442);
  parkingSpaces[7].polyPoints[1] = Point(526,445);
  parkingSpaces[7].polyPoints[2] = Point(527,466);
  parkingSpaces[7].polyPoints[3] = Point(469,464);
  parkingSpaces[8].polyPoints[0] = Point(530,279);
  parkingSpaces[8].polyPoints[1] = Point(591,281);
  parkingSpaces[8].polyPoints[2] = Point(591,303);
  parkingSpaces[8].polyPoints[3] = Point(531,300);
  parkingSpaces[9].polyPoints[0] = Point(533,447);
  parkingSpaces[9].polyPoints[1] = Point(593,449);
  parkingSpaces[9].polyPoints[2] = Point(593,471);
  parkingSpaces[9].polyPoints[3] = Point(533,467);


  // set centroids of parkingSpaces
  for(int i = 0; i<10 ; i++)
  {
    Point centroid = Point(0,0);
    for (int j = 0; j<4 ; j++)
    {
      centroid += parkingSpaces[i].polyPoints[j];
    }
    centroid = centroid/4;
    parkingSpaces[i].centroid = centroid;
  }

  // ***************************   HARDCODED MAP *********************************
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
      //Prints parking user interface
      printParking();

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
          case 't':
              // Toggle training functionality
              trainingActive = !trainingActive;
              if(trainingActive)
              {
                if(segSeeds.size()!=1)
                {
                  cout<< "Error: solo debe haber una semilla en el entrenamiento" << endl;
                  trainingActive = false;
                  break;
                }
                else
                {
                  // print menu to pick training figure
                  cout << "Que figura desea entrenar? "<<endl;
                  cout << "  [0] BB8"<<endl;
                  cout << "  [1] Lightsaber "<<endl;
                  cout << "  [2] R2D2 "<<endl;
                  cout << "  [3] Spaceship "<<endl;
                  cin >> trainingId;
                  // Clear phi training vector from figure
                  trainedData[trainingId].fiTVector.clear();
                }
              }
              else
              {
                cout << "TrainingId: " << trainingId << endl;
                calculateTrainedPhi();
              }
              break;
          case 'p':
              // Push training moment to currently trained figure
              if(trainingActive){
                trainedData[trainingId].fiTVector.push_back(fiTrainingPair);
                cout<< "MOMENT PUSHED FOR TRAINING" <<endl;
              }
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
  moveWindow("Segmentos", 1300, 0);
  // Print contours
  imshow("Contornos", contoursMat);
  moveWindow("Contornos", 0, 0);
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
	//currentImage.copyTo(output, hsv_thres);

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

	//imshow("HSV Threshold", output);
  //moveWindow("HSV Threshold", 1300, 0);
	//output.release();
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
  int height=350, width=350;
  int cRadius=160;
  int c1=1, c2=1, c3=1, c4=1;
  // create mat object to draw on
  Mat eye(height, width, CV_8UC3);
  //paint it white
  eye.setTo(Scalar(255,255,255));

  //Fill respective quadrant
  switch (cuadrante){
    case 1:
      c1 = -1;
      puntoInicio = Point(700,53);
      break;
    case 2:
      c2 = -1;
      puntoInicio = Point(89,0);
      break;
    case 3:
      c3 = -1;
      puntoInicio = Point(0,504);
      break;
    case 4:
      c4 = -1;
      puntoInicio = Point(622,558);
      break;
    default:;
  }

  //draw black axis
  line(eye, Point(0, height/2), Point(width, height/2), Scalar( 0, 0, 0 ), 1, 8, 0);
  line(eye, Point(width/2, 0), Point(width/2, height), Scalar( 0, 0, 0 ), 1, 8, 0);

  //draw circle quarters
  ellipse(eye, Point(width/2, height/2), Size(cRadius, cRadius), 0, 0, 90, Scalar( 0, 0, 255 ), c4, 8, 0);
  ellipse(eye, Point(width/2, height/2), Size(cRadius, cRadius), 0, 90, 180, Scalar( 0, 0, 255 ), c3, 8, 0);
  ellipse(eye, Point(width/2, height/2), Size(cRadius, cRadius), 0, 180, 270, Scalar( 0, 0, 255 ), c2, 8, 0);
  ellipse(eye, Point(width/2, height/2), Size(cRadius, cRadius), 0, 270, 360, Scalar( 0, 0, 255 ), c1, 8, 0);

  //Draw angle line
  int angleX = cRadius * cos((-angle*pi)/180);
  int angleY = cRadius * sin((-angle*pi)/180);
  line(eye, Point(width/2+angleX, height/2+angleY), Point(width/2-angleX, height/2-angleY), Scalar( 255, 0, 0), 1, CV_AA, 0);

  putText(eye, "BB8", Point(width * 0.7, height * 0.52), FONT_HERSHEY_PLAIN, 1, Scalar( 0, 0, 0 ), 2, CV_AA, 0);
  putText(eye, "Light", Point(width * 0.45, height * 0.3), FONT_HERSHEY_PLAIN, 1, Scalar( 0, 0, 0 ), 2, CV_AA, 0);
  putText(eye, "R2D2", Point(width * 0.2, height * 0.52), FONT_HERSHEY_PLAIN, 1, Scalar( 0, 0, 0 ), 2, CV_AA, 0);
  putText(eye, "Spaceship", Point(width * 0.4, height * 0.75), FONT_HERSHEY_PLAIN, 1, Scalar( 0, 0, 0 ), 2, CV_AA, 0);

  imshow("bullsEye", eye);
  //moveWindow("bullsEye", 800, 0);
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
  //normal operation
  if(!trainingActive)
  {
    find_moments(segmentToMomento, myfile);
  }
  else
  {
    updateTrainingMoment(segmentToMomento);
  }
}

int cuentame = 0;
/* --------------------------------------------------------------------------------------*
 * Function to calculate Hu Moments 									 	                                 *
 * ------------------------------------------------------------------------------------- */
void find_moments(Mat segmentMoment, ofstream &myfile)
{
		Mat canny_output;
    Mat drawing = Mat::zeros( hsv_thres.size(), CV_8UC3 ); // Draw contours
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

	for( int i = 0; i< (int)contours.size(); i++ )
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

  centroid = Point2f(mu.m10/huMoments[3] , mu.m01/huMoments[3] );

  drawContours(drawing, contours, 0, colors[0], 2, 8, hierarchy, 0, Point());
  circle(drawing, centroid, 4, colors[1], -1, 8, 0);

  // Orientation
  angle_rad = 0.5*atan2( (2*huMoments[2])  , (huMoments[0]- huMoments[1]) );
  angle_g = angle_rad * (180.00/3.1416);

  // Draw line (orientation) //TODO: Complete line
  P1 = centroid;
  double l = 250;
  P2.x = P1.x + l * sin(angle_rad+3.1416/2);
  P2.y = P1.y - l * cos(angle_rad+3.1416/2);
  line(drawing, P1, P2, colors[2], 1, 8, 0);

  P2.x = P1.x - l * sin(angle_rad+3.1416/2);
  P2.y = P1.y + l * cos(angle_rad+3.1416/2);
  line(drawing, P1, P2, colors[2], 1, 8, 0);

  // Show in a window
  imshow("Centroid with Axis", drawing );
  moveWindow("Centroid with Axis", 1300, 0);


  //cout << "angulo rad: " << angle_rad << endl;
  //cout << "angulo grado: " << angle_g << endl;

  printf("Fi1: %f \n", fi[0]);
  printf("Fi2: %f \n", fi[1]);

  //myfile << fi[0] << "," << fi[1] << endl;


  // ***************************   Calc distance in Phis to figure *********************************

  double ddbb8 = pow((fi[0] - trainedData[0].fi1),2) + pow((fi[1]-trainedData[0].fi2),2);
  double ddea = pow((fi[0] - trainedData[1].fi1),2) + pow((fi[1]-trainedData[1].fi2),2);
  double ddr2 = pow((fi[0] - trainedData[2].fi1),2) + pow((fi[1]-trainedData[2].fi2),2);
  double ddnave = pow((fi[0] - trainedData[3].fi1),2) + pow((fi[1]-trainedData[3].fi2),2);

  double dmin = min(ddea,ddbb8);
  dmin = min(dmin, ddr2);
  dmin = min(dmin, ddnave);



  //Reset angle and figure presence for bullsEye drawing
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
    if  ( ( ( trainedData[0].fi1- trainedData[0].desv1 ) < fi[0] ) &&  ( fi[0]  < (trainedData[0].fi1 + trainedData[0].desv1 )  )  )
    {
      if ( ( (trainedData[0].fi2 - trainedData[0].desv2) < fi[1] ) && (fi[1]  < (trainedData[0].fi2 + trainedData[0].desv2 ) ) )
      {
        cout << "bb8" << endl;
        presentFigures[BB8] = true; // BB8 present on screen
      }
    }
  }
  else if (dmin == ddea)
  {
    if (( (trainedData[1].fi1- 5*trainedData[1].desv1) < fi[0]) &&     (fi[0]< (trainedData[1].fi1 + 5*trainedData[1].desv1) ) )
    {
      if (( (trainedData[1].fi2- 5*trainedData[1].desv2) < fi[1]) &&  (fi[1]< (trainedData[1].fi2 + 5*trainedData[1].desv2) ) )
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
    if (( (trainedData[2].fi1- 5*trainedData[2].desv1) < fi[0]) && (fi[0]< (trainedData[2].fi1 + 5*trainedData[2].desv1) ) )
    {
      if (( (trainedData[2].fi2- 5*trainedData[2].desv2) < fi[1]) && (fi[1]< (trainedData[2].fi2 + 5*trainedData[2].desv2) ) )
        {
          cout << "r2" << endl;
          presentFigures[R2D2] = true; // R2D2 present on screen
        }
    }
  }
  else if (dmin == ddnave)
  {
    if (( (trainedData[3].fi1- 5*trainedData[3].desv1) < fi[0]) && (fi[0]< (trainedData[3].fi1 + 5*trainedData[3].desv1) ) )
    {
      if (( (trainedData[3].fi2- 5*trainedData[3].desv2) < fi[1]) && (fi[1]< (trainedData[3].fi2 + 5*trainedData[3].desv2) ) )
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

  double scalingFactorX = hsv_thres.cols * 1.75;
  double scalingFactorY = hsv_thres.rows * 4.5;

  // Draw Fi values 320x240
  Point xi, xf, yi, yf, textf1, textf2;
  xi.x = 0; xi.y = hsv_thres.rows;
  xf.x = hsv_thres.cols; xf.y = hsv_thres.rows;
  yi.x = 0; yi.y = 0;
  yf.x = 0; yf.y = hsv_thres.rows;

  textf1.x = hsv_thres.cols - 45; textf1.y = hsv_thres.rows - 10;
  textf2.x = 10; textf2.y = 30;

  line(plano, xi, xf, color_line, 2, 8, 0);
  line(plano, yi, yf, color_line, 2, 8, 0);

  putText(plano, "Fi1", textf1, FONT_HERSHEY_SIMPLEX, 1, color_point, 1, 2, 0);
  putText(plano, "Fi2", textf2, FONT_HERSHEY_SIMPLEX, 1, color_point, 1, 2, 0);
  // Plotear fi values
  Point P_fi;
  P_fi.x = (fi[0] * scalingFactorX); //Fi1
  P_fi.y = (fi[1] * scalingFactorY); //Fi2
  circle(plano, Point(P_fi.x, hsv_thres.rows - P_fi.y), 0, color_point, 5, 8, 0);

  //Paint stored figures caracterization

  //trainedData[trainingId].fi2 = mean;
  //trainedData[trainingId].desv2 = standardDeviation;

  for(int i = 0; i<4 ; i++){
    //rectangle(plano, Point((trainedData[i].fi2 - trainedData[i].desv2)* hsv_thres.cols, (trainedData[i].fi1 + trainedData[i].desv1)* hsv_thres.rows), Point((trainedData[i].fi2 + trainedData[i].desv2)* hsv_thres.cols, (trainedData[i].fi1 - trainedData[i].desv1)* hsv_thres.rows), colors[i], 2, 8, 0);
    rectangle(plano, Point((trainedData[i].fi1 + trainedData[i].desv1)* scalingFactorX, hsv_thres.rows - (trainedData[i].fi2 - trainedData[i].desv2)* scalingFactorY), Point((trainedData[i].fi1 - trainedData[i].desv1)* scalingFactorX, hsv_thres.rows - (trainedData[i].fi2 + trainedData[i].desv2)* scalingFactorY), colors[i], 2, 8, 0);
    // Plot every training point for current figure
    for(myPiStruct trainingF : trainedData[i].fiTVector)
    {
      circle(plano, Point(trainingF.phiG1* scalingFactorX, hsv_thres.rows - trainingF.phiG2*scalingFactorY), 1, colors[i], -1, 8, 0);
    }
  }

  putText(plano, "BB8", Point(trainedData[0].fi1* scalingFactorX, hsv_thres.rows - trainedData[0].fi2 * scalingFactorY), FONT_HERSHEY_SIMPLEX, 0.5, color_point, 1, 2, 0);
  putText(plano, "Light", Point(trainedData[1].fi1* scalingFactorX, hsv_thres.rows - trainedData[1].fi2 * scalingFactorY), FONT_HERSHEY_SIMPLEX, 0.5, color_point, 1, 2, 0);
  putText(plano, "R2D2", Point(trainedData[2].fi1* scalingFactorX, hsv_thres.rows - trainedData[2].fi2 * scalingFactorY), FONT_HERSHEY_SIMPLEX, 0.5, color_point, 1, 2, 0);
  putText(plano, "Spaceship", Point(trainedData[3].fi1* scalingFactorX, hsv_thres.rows - trainedData[3].fi2 * scalingFactorY), FONT_HERSHEY_SIMPLEX, 0.5, color_point, 1, 2, 0);
  namedWindow("Plano", CV_WINDOW_AUTOSIZE);
  imshow("Plano", plano);
  moveWindow("Plano", 0, 1000);
}

/* --------------------------------------------------------------------------------------*
 * Function to calculate one Hu Moments 									 	                                 *
 * ------------------------------------------------------------------------------------- */
void updateTrainingMoment(Mat segmentMoment)
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

	if (contours.size() > 0)
  {

  	for( int i = 0; i< (int)contours.size(); i++ )
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
    fiTrainingPair.phiG1 = nMoments[0] + nMoments[1]; //Fi1
    fiTrainingPair.phiG2 = pow((nMoments[0] - nMoments[1]), 2) + 4*pow(nMoments[2], 2); //Fi2
  }
}

/* --------------------------------------------------------------------------------------*
 * Function to update trainedData with phi vector									 	                                 *
 * ------------------------------------------------------------------------------------- */
void calculateTrainedPhi()
{
    int sizeOfVector = trainedData[trainingId].fiTVector.size();
    double sum = 0.0, mean, standardDeviation = 0.0;
    if(sizeOfVector > 0)
    {
      for(int i = 0; i < sizeOfVector; i++)
      {
          sum += trainedData[trainingId].fiTVector.at(i).phiG1;
      }

      mean = sum/sizeOfVector;

      for(int i = 0; i < sizeOfVector; i++)
          standardDeviation += pow(trainedData[trainingId].fiTVector.at(i).phiG1 - mean, 2);

      standardDeviation = sqrt(standardDeviation / sizeOfVector);

      trainedData[trainingId].fi1 = mean;
      trainedData[trainingId].desv1 = standardDeviation;

      sum = 0.0;
      standardDeviation = 0.0;

      for(int i = 0; i < sizeOfVector; i++)
      {
          sum += trainedData[trainingId].fiTVector.at(i).phiG2;
      }

      mean = sum/sizeOfVector;

      for(int i = 0; i < sizeOfVector; i++)
          standardDeviation += pow(trainedData[trainingId].fiTVector.at(i).phiG2 - mean, 2);

      standardDeviation = sqrt(standardDeviation / sizeOfVector);

      trainedData[trainingId].fi2 = mean;
      trainedData[trainingId].desv2 = standardDeviation;
    }
}


/* --------------------------------------------------------------------------------------*
 * Function to print the parking spaces user interface			 					 	                                 *
 * ------------------------------------------------------------------------------------- */
void printParking()
{

    imshow("Estacionamiento", parkingOriginal);
    setMouseCallback("Estacionamiento", mouseOnParkingImage);
}


/* --------------------------------------------------------------------------------------*
 * Function to return navigation base 1 channel matrix depending on target parking space			 					 	                                 *
 * ------------------------------------------------------------------------------------- */
Mat getNavigationBase(int parkingTargetIndex)
{
  Mat returnMat = Mat::zeros( parkingOriginal.rows, parkingOriginal.cols, CV_8UC1 );
  parkingLimits_1channel.copyTo(returnMat);
  fillConvexPoly(returnMat, parkingSpaces[parkingTargetIndex].polyPoints, 4, 0, CV_AA, 0);
  return returnMat;
}

/* --------------------------------------------------------------------------------------*
 * Callback for click on Parking Space window 			 					 	                                 *
 * ------------------------------------------------------------------------------------- */
void mouseOnParkingImage(int event, int x, int y, int flags, void* param)
{
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
            //Test that parking original is loaded
            if(parkingOriginal.rows > 0)
            {
              Mat parkingTargets = Mat::zeros( parkingOriginal.rows, parkingOriginal.cols, CV_8UC1 );

              for(int i = 0; i<10 ; i++)
              {
                fillConvexPoly(parkingTargets, parkingSpaces[i].polyPoints, 4, i+1, CV_AA, 0);
              }

  						// Get parking space index at point
  						int index = parkingTargets.at<uchar>(y,x) - 1;

              if(index>-1)
              {
                // Print mouse position
                //cout << "  Mouse X, Y: " << x << ", " << y << " ---> ";
                cout << "Parking Space Number: " << index <<endl;
                Mat navigationBase = Mat::zeros( parkingOriginal.rows, parkingOriginal.cols, CV_8UC1 );
                navigationBase = getNavigationBase(index);
                imshow("navigationBase", navigationBase);
              }
              else
              {
                cout<< "HACER CLICK EN ESPACIO DE ESTACIONAMIENTO LIBRE" <<endl;
              }

            }
            break;
        default:
            break;
    }
}
  //Mapa original , //Punto de inicio
  list create_NF1(Mat parkingOriginal, Point Pi,Point punto_partida ){

  //MAT auxiliar for harcoded Map
  Mat inicial = Mat::zeros( parkingOriginal.rows, parkingOriginal.cols, CV_64F);
  Point matriz  = Point(parkingOriginal.rows,parkingOriginal.cols);
  matriz = matriz + Point(-1,-1); //Limits for the matrix
  double valor_celda;

  for(int i=0; i<= parkingOriginal.rows ; i++){
    for(int j=0; i<=parkingOriginal.cols; j++){
      double valor_celda = inicial.at<double>(i,j);
      if (valor_celda=255){
        inicial.at<double>(i,j) = INT_MAX;
      }

    }
  }

  cout<< inicial << endl; //Matriz base
  list<Point> loc; //List with locations
  double current=0;
  cout<< inicial<<endl;
  cout <<"\n";
  cout <<"\n";
  Point vecinos[4] = {Point(0,1),Point(0,-1),Point(1,0),Point(-1,0)}; //Coordenadas vecinos
  Point Padj,Pev; //Punto evaluado //Punto adyacente
  
  loc.push_back(Pi); //Se implanta semilla - Punto inicial

  while (!loc.empty()){
    //Pev = Fo.back(); 
    Pev = loc.front(); 
    double current = inicial.at<double>(Pev.x,Pev.y);
    //Fo.pop_back();
    loc.pop_front();


    for(int i = 0; i < 4 ; i++){
      Padj = Pev + vecinos[i];
      double valor_celda = inicial.at<double>(Padj.x,Padj.y); // Valores en el punto adjacente      
      //  if (Color(Pa)=0 and f(Pa)=1)

      //if( valor_celda==0 && Padj!=Pi &&Padj.x<=matriz.x &&Padj.x>=0 && Padj.y <=matriz.y && Padj.y >=0)
      if( valor_celda == 0 && Padj!=Pi && Padj.x<=matriz.x && Padj.x>=0 && Padj.y<=matriz.y && Padj.y >=0)
      {
          //Segment for colorfull mat feedback
          inicial.at<double>(Padj.x,Padj.y) = current +1;
          loc.push_back(Padj);  
          
      }
    }
  }


  /* BUSQUEDA*/
  list<Point> path;
  Point Padjacente;
  double valor_vecino;
  double valor_actual;

  path.push_back(punto_partida);

  valor_actual = inicial.at<double>(punto_partida.x,punto_partida.y);
  cout<< valor_actual<<endl;
  while (valor_actual!=0){

    for(int i = 0; i < 4 ; i++){
      Padjacente = punto_partida + vecinos[i];
      double valor_vecino = inicial.at<double>(Padjacente.x,Padjacente.y); // Valores en el punto adjacente      

      if( Padjacente!=punto_partida &&Padjacente.x<=matriz.x &&Padjacente.x>=0 && Padjacente.y <=matriz.y && Padjacente.y >=0)
      {
        if (valor_vecino<valor_actual){
        punto_partida = Padjacente;
        valor_actual = inicial.at<double>(punto_partida.x,punto_partida.y);
        }  
      path.push_back(punto_partida);    
      }
    }
  }
return path;
}


void pintar_ruta(list path, Mat imagen){
  Point previo;
  while(!path.empty()){
    previo = path.pop_front();
    line(imagen,previo,path.front(), Vec3i(255, 244, 30) , 1) //Azul
  }
  imshow("final",imagen);
}



/* --------------------------------------------------------------------------------------*
 * Function to return navigation base 1 channel matrix depending on target parking space			 					 	                                 *
 * ------------------------------------------------------------------------------------- *//*
void calcularRuta()
{
  getNavigationBase()
}
