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


float matHu[2][3]; //Stores Phi1 and Phi2 of the pictureype
RNG rng(12345);

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


/* This is the callback that will only display mouse coordinates */
void mouseCoordinatesExampleCallback(int event, int x, int y, int flags, void* param);


//find moments
void find_moments(Mat bin,ofstream &myfile);

/* Create images where captured and transformed frames are going to be stored */
Mat currentImage;

/* Create images where captured and transformed frames are going to be stored */
Mat auxImage;
Mat hsv_thres;
Mat hsv2_thres;
Mat GUI;
Mat output;

//Merging of both masks
Mat res;
// Create image template for coloring
Mat solidColor(50, 50, CV_8UC3);

//contador de veces que se seleccionan colores
int cont= 0;

int contador=0;

string name;


//double r2promfi1 = 0.185680906;
//double r2promfi2 = 0.005930427;
//double r2varf1 = 0.000198173;
//double r2varf2 = .00000706918;

//double r2des1 = 0.018774486;
//double r2des2 = 0.002640373;



//ARTU con MOMENTOS EN contorno
double r2promfi1 = 0.18647124;
double r2promfi2 = 0.007369524;
double r2varf1 = 0.0000700905;
double r2varf2 = 0.00000784067;

double r2des1 = 0.018774486;
double r2des2 = 0.002640373;



//el que sale como artu 
//double esppromfi1 = 0.465996405;
//double esppromfi2 = 0.126943452;
//double espvarf1 = 0.004179452;
//double espvarf2 = 0.000547676;

//double espdesf1 = 0.064648681;
//double espdesf2 = 0.023402486;


///prueba biblio definitivo artu
//double esppromfi1 = 0.7030579;
//double esppromfi2 = 0.54781283;
//double espvarf1 = 0.09975878;
//double espvarf2 = 0.11375427;

//double espdesf1 = 0.064648681;
//double espdesf2 = 0.023402486;


double esppromfi1 = 0.36159965;
double esppromfi2 = 0.11202307;
double espvarf1 = 0.01774462;
double espvarf2 = 0.0100457;

double espdesf1 = 0.064648681;
double espdesf2 = 0.023402486;




void mod_HSV_threshold(int,void*,ofstream &myfile){
	cvtColor(currentImage, auxImage, cv::COLOR_BGR2HSV);
	

	inRange(auxImage, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), hsv_thres);


	///////////*********DIlation*********************** For filtering masks
    //https://docs.opencv.org/2.4/doc/tutorials/imgproc/opening_closing_hats/opening_closing_hats.html
    // Since MORPH_X : 2,3,4,5 and 6
    //operation "Operator:\n 0: Opening - 1: Closing \n 2: Gradient - 3: Top Hat \n 4: Black Hat"
  	int operation = 3;
  	int morph_size = 5;

  	Mat element = getStructuringElement( 0, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
  	/// Apply the specified morphology operation
  	//blur( hsv_thres, hsv_thres, Size(5,5) );
  	morphologyEx( hsv_thres, hsv_thres, operation, element );


  	operation = 2;
  	morph_size = 10;
  	element = getStructuringElement( 0, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
  	morphologyEx( hsv_thres, hsv_thres, operation, element );


  	operation = 3;
  	morph_size = 5;
  	element = getStructuringElement( 0, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
  	morphologyEx( hsv_thres, hsv_thres, operation, element );



  	find_moments(hsv_thres, myfile);


  	//aqui va lo del bloc de notas

	
	
	imshow("HSV Thres", hsv_thres);
	currentImage.copyTo(output, hsv_thres);

	imshow("HSV", output);
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

	ofstream myfile;
    	myfile.open ("nothing.csv");
    	myfile << "fi1,fi2,name\n";




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
          			mod_HSV_threshold(0,0,myfile);
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
					createTrackbar("Hmin", "HSV", &low_H, 255);
					createTrackbar("Hmax", "HSV", &high_H, 255);
					createTrackbar("Smin", "HSV", &low_S, 255);
					createTrackbar("Smax", "HSV", &high_S, 255);
					createTrackbar("Vmin", "HSV", &low_V, 255);
					createTrackbar("Vmax", "HSV", &high_V, 255);;
					//mod_HSV_threshold(0,0,myfile);
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
        	cont+=1;
        	if (cont>1){
        		cont=0;
        	}
			getParameterRange();
			points.clear();
            break;
    }
}






/* --------------------------------------------------------------------------------------*
 * Funcion para encontrar Hu Moments 									 	             *
 * -------------------------------------------------------------------------------------- */
void find_moments(Mat bin, ofstream &myfile) {
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	
	// Detect edges using canny
	Canny(bin, canny_output, 50, 150, 3 );
	
	// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) ); //CV_RETR_TREE

	int largest_area = 0;
	int largest_contour_index = 0;

	for( int i = 0; i< contours.size(); i++ ) {
	//Find the area of contour
        double a=contourArea( contours[i],false);
        //if a> largest_area
        if(a>largest_area){
            
        	//Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
            //drawContours( drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );

            largest_area=a;
            // Store the index of largest contour
            largest_contour_index=i;               
        }

        //else{
        //	if (a>secondlargest_area){
        //		secondlargest_area = a;
        //		secondlargest_contour_index=i;
        //	}
    }

	
	
	vector<Moments> mu(contours.size());                      // Moments
	double huMoments[contours.size()][3];	                  // Hu Moments
	double nMoments[contours.size()][3];					  // N Moments
	double gamma[3];
	gamma[0] = ((2+0)/2) + 1; 								  // Gamma 20
	gamma[1] = ((0+2)/2) + 1; 								  // Gamma 02
	gamma[2] = ((1+1)/2) + 1; 								  // Gamma 11
	double fi[contours.size()][2];							  // Fi Values
	vector<Point2f> centroid( contours.size() ); 			  // Centroids
	Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 ); // Draw contours
	Mat plano = Mat::zeros( canny_output.size(), CV_8UC3 );
	double angle_rad, angle_g;								  // Orientation with Hu moments
	double angles_g[contours.size()];
	double angles_rad[contours.size()];
	// Draw line (orientation)
	Scalar color_line = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	Scalar color_point = Scalar( rng.uniform(255, 255), rng.uniform(255,255), rng.uniform(255,255) );
	Point2f P2, P1;
	// Calculate the area with the moments 00 and compare with the result of the OpenCV function
	double area[contours.size()];
	double length[contours.size()];
	int fig = 0; 
	
	for( int i = 0; i< contours.size(); i++ ) {

		double o=contourArea( contours[largest_contour_index],false);
		double e=contourArea( contours[i],false);
		if  (  e > (o-30) ){


		// Get Moments
		mu[i] = moments(contours[i], false);
		
		// Get Hu Moments
		huMoments[i][0] = mu[i].m20 - ((mu[i].m10/mu[i].m00)*mu[i].m10); //Mu20
		huMoments[i][1] = mu[i].m02 - ((mu[i].m01/mu[i].m00)*mu[i].m01); //Mu02
		huMoments[i][2] = mu[i].m11 - ((mu[i].m01/mu[i].m00)*mu[i].m10); //Mu11
		huMoments[i][3] = mu[i].m00;                                     //Mu00
		
		// Get N Moments
		nMoments[i][0] = huMoments[i][0] / (pow(mu[i].m00, gamma[0])); //n20
		nMoments[i][1] = huMoments[i][1] / (pow(mu[i].m00, gamma[1])); //n02
		nMoments[i][2] = huMoments[i][2] / (pow(mu[i].m00, gamma[2])); //n11
		
		// Get Fi values
		fi[i][0] = nMoments[i][0] + nMoments[i][1]; //Fi1 
		fi[i][1] = pow((nMoments[i][0] - nMoments[i][1]), 2) + 4*pow(nMoments[i][2], 2); //Fi2
		
		// Get centroids
		centroid[i] = Point2f(mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
		
		// Draw contours
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
		circle(drawing, centroid[i], 4, color, -1, 8, 0);
		
		// Orientation 
		//angle_rad = 0.5*atan((2*huMoments[i][2])/(huMoments[i][0]-huMoments[i][1]));
		angle_rad = 0.5*atan2( (2*huMoments[i][2])  , (huMoments[i][0]- huMoments[i][1]) );
		angle_g = angle_rad * (180.00/3.1416);
		angles_g[i] = angle_g;
		angles_rad[i] = angle_rad;
		

		cout << "angulo rad: " << angles_rad[i] << endl;
		cout << "angulo grado: " << angles_g[i] << endl;

		
		// Draw line (orientation) //TODO: Complete line
		P1 = centroid[i];
		double l = 100;
		P2.x = P1.x + l * sin(angles_rad[i]);
		P2.y = P1.y - l * cos(angles_rad[i]);
		line(drawing, P1, P2, color_line, 5, 8, 0);

		P2.x = P1.x - l * sin(angles_rad[i]);
		P2.y = P1.y + l * cos(angles_rad[i]);
		line(drawing, P1, P2, color_line, 5, 8, 0);
		
		// Show in a window
		namedWindow("6. Moments Image", CV_WINDOW_AUTOSIZE);
		imshow("6. Moments Image", drawing );
		moveWindow("6. Moments Image", 1000, 400);
		
		area[i] = mu[i].m00;
		length[i] = arcLength(contours[i], true);
		//printf(" * Figura[%d] - Area (M_00): %.2f - Length: %.2f - Angle: %.2f \n", i, area[i], length[i], angles_g[i]);
		
		
		if (area[i] > 400) {
			// Print moments
			//printf("HM00: %.2f \n", huMoments[i][3]);
			//printf("HM11: %.2f \n", huMoments[i][2]);
			//printf("HM02: %.2f \n", huMoments[i][1]);
			//printf("HM20: %.2f \n", huMoments[i][0]);
			printf("Fi1: %f \n", fi[i][0]);
			printf("Fi2: %f \n", fi[i][1]);
			
			//ofstream moments; 
			//moments.open("casahueca.txt",  std::ofstream::out | //std::ofstream::app);
			//moments << "2," << fi[i][0] << "," << fi[i][1] << endl; 
			//moments.close(); 
			double fi1 = fi[i][0];
			double fi2 = fi[i][1];

			myfile << fi[i][0] << "," << fi[i][1] << endl; 


			double fi1_bb8 = 0.179271142;
			double fi2_bb8 = 0.004877734;
			double desv1_bb8 = 0.011026558;
			double desv2_bb8 = 0.005050658;

			double ddbb8 = pow((fi1 - fi1_bb8),2) + pow((fi2-fi2_bb8),2);


			double fi1_ea = 0.48499072;
			double fi2_ea = 0.19502295;
			double desv1_ea = 0.01722625;
			double desv2_ea = 0.01254551;

			double ddea = pow((fi1 - fi1_ea),2) + pow((fi2-fi2_ea),2);



			double fi1_r2 = 0.197311215;
			double fi2_r2 = 0.013102077;
			double desv1_r2 = 0.002949608;
			double desv2_r2 = 0.001129111;

			double ddr2 = pow((fi1 - fi1_r2),2) + pow((fi2-fi2_r2),2);



			double dmin = min(ddea,ddbb8);
			dmin = min(dmin, ddr2);
			bool flagzaso = false;

			if (dmin == ddbb8){
				//if  ( ( ( fi1_bb8- .3 ) < fi1 ) &&  ( fi1  < (fi1_bb8 + .3 )  )  ) {
				if  ( ( ( fi1_bb8- desv1_bb8 ) < fi1 ) &&  ( fi1  < (fi1_bb8 + desv1_bb8 )  )  ) {
					//if ( ( (fi2_bb8 - .003) < fi2 ) &&  (fi2  < (fi2_bb8 + .003 ) ) ){
					if ( ( (fi2_bb8 - desv2_bb8) < fi2 ) &&  (fi2  < (fi2_bb8 + desv2_bb8 ) ) ){
						cout << "bb8" << endl;
						flagzaso = true;
					}

				}
			} else if (dmin == ddea){
				if (( (fi1_ea- 5*desv1_ea) < fi1) &&     (fi1< (fi1_ea + 5*desv1_ea) ) ) {
					if (( (fi2_ea- 5*desv2_ea) < fi2) &&  (fi2< (fi2_ea + 5*desv2_ea) ) ){
						cout << "espadux" << endl;
						flagzaso = true;
					}

				}
			}else if (dmin == ddr2){
				if (( (fi1_r2- 5*desv1_r2) < fi1) &&     (fi1< (fi1_r2 + 5*desv1_r2) ) ) {
					if (( (fi2_r2- 5*desv2_r2) < fi2) &&  (fi2< (fi2_r2 + 5*desv2_r2) ) ){
						cout << "r2" << endl;
						flagzaso = true;
					}

				}
			}

			if (!flagzaso){
				cout <<"nada ft. el chido" <<endl;
			}
			
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
			P_fi.x = (fi[i][0] * 320); //Fi1
			P_fi.y = (fi[i][1] * 240); //Fi2
			circle(plano, P_fi, 0, color_point, 15, 8, 0);
		}
		namedWindow("Plano", CV_WINDOW_AUTOSIZE);
		imshow("Plano", plano);
	}
}}

