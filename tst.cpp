#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

using namespace cv;
/// Global variables

Mat src, src_gray;
Mat dst;

void circleDetection(int, void*)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	Mat circleImage;

	//Binarization
	threshold( src_gray, src_gray, 145, 255, THRESH_BINARY_INV );
	//imwrite("coinsThreshold.png", src_gray);

	//Blur the image so we can avoid noise
	blur( src_gray, src_gray, Size(5,5) );
	//imwrite("coinsBlur.png", src_gray);

	//Erosion followed by dilation
	morphologyEx(src_gray, src_gray, MORPH_OPEN , getStructuringElement(MORPH_RECT, Size(5,5), Point(-1,-1)),Point(-1,-1), 13);
	//imwrite("coinsEroDil.png", src_gray);

	vector<Vec3f> circles;
	HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, 220, 50, 20, 100, 165 );

	//char* name = "../../images/Gray_Image";
	char buffer[40];
	char name[100];
	/// Draw the circles detected
	for( size_t i = 0; i < circles.size(); i++ )
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);

		cv::Mat eyeImg = src(cv::Rect(center.x - radius, // ROI x-offset, left coordinate
								center.y - radius, // ROI y-offset, top coordinate 
								2*radius,          // ROI width
								2*radius));        // ROI height


		sprintf(buffer, "%ld", i);
		strcpy(name, "CoinsImage");
		strcat(name,buffer);
		strcat(name, ".JPG");

		std::cout<<name<<"\n";
				
		imwrite( name, eyeImg );

		memset(buffer, 0, 40);
		memset(name,0,100);

		// circle center
		circle( src, center, 3, Scalar(255,0,0), -1, 8, 0 );

		//circle outline
		circle( src, center, radius, Scalar(255,0,0), 3, 8, 0 );
	}
	//imwrite("coins.png", src);
	imshow( "Coin Detector", src );
}


int main( int argc, char** argv )
{
	/// Load an image
	src = imread( argv[1] );

	if( !src.data )
	{ return -1; }

	/// Create a matrix of the same type and size as src (for dst)
	dst.create( src.size(), src.type() );

	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );

	/// Create a window
	namedWindow( "Coin Detector", CV_WINDOW_NORMAL );

	/// Detect the circles and show the image
	circleDetection(0, 0);

	/// Wait until user exit program by pressing a key
	waitKey(0);

	return 0;
}
