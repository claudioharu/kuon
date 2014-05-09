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
MatND hist_base[6];
int baseRadius[6];
int Radius[100];

long int numCoins;

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

	char buffer[40];
	char name[100];
	/// Draw the circles detected
	for( size_t i = 0; i < circles.size(); i++ )
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		Radius[i] = cvRound(circles[i][2]);

		cv::Mat eyeImg;
		
		try
		{
			eyeImg = src(cv::Rect(center.x - radius, // ROI x-offset, left coordinate
									center.y - radius, // ROI y-offset, top coordinate 
									2*radius,          // ROI width
									2*radius));        // ROI height
		}
		catch (cv::Exception e)
		{
			
			radius -= 10;
			eyeImg = src(cv::Rect(center.x - radius, // ROI x-offset, left coordinate
									center.y - radius, // ROI y-offset, top coordinate 
									2*radius,          // ROI width
									2*radius));        // ROI height
		}

		sprintf(buffer, "%ld", i);
		strcpy(name, "CoinsImage");
		strcat(name,buffer);
		strcat(name, ".JPG");

		//std::cout<<name<<"\n";
				
		imwrite( name, eyeImg );

		memset(buffer, 0, 40);
		memset(name,0,100);
		numCoins = i;
		// circle center
		//circle( src, center, 3, Scalar(255,0,0), -1, 8, 0 );

		//circle outline
		//circle( src, center, radius, Scalar(255,0,0), 3, 8, 0 );
	}
	
	
	//imwrite("coins.png", src);
	imshow( "Coin Detector", src );
}

void baseHistogram()
{
	Mat src_base, hsv_base;
	/// Histograms
	
	char name[100];
	char buffer[40];
	
	for(int i = 0; i < 5; i++)
	{
		sprintf(buffer, "%d", i);
		strcpy(name, "Coin");
		strcat(name,buffer);
		strcat(name, ".JPG");
		
		src_base =imread( name );
		
		//std::cout << name <<"\n";
		
		memset(buffer, 0, 40);
		memset(name,0,100);
				
		/// Convert to HSV
		cvtColor( src_base, hsv_base, COLOR_BGR2HSV );

		/// Using 50 bins for hue and 60 for saturation
		int h_bins = 50; int s_bins = 60;
		int histSize[] = { h_bins, s_bins };

		// hue varies from 0 to 179, saturation from 0 to 255
		float h_ranges[] = { 0, 180 };
		float s_ranges[] = { 0, 256 };

		const float* ranges[] = { h_ranges, s_ranges };

		// Use the o-th and 1-st channels
		int channels[] = { 0, 1 };

		/// Calculate the histograms for the HSV images
		calcHist( &hsv_base, 1, channels, Mat(), hist_base[i], 2, histSize, ranges, true, false );
		normalize( hist_base[i], hist_base[i], 0, 1, NORM_MINMAX, -1, Mat() );
		
	}
	
}

void compareHistogram()
{
	char buffer[40];
	char name[100];
	double similarity[numCoins+1];
	
	for(long int i = 0; i <numCoins+1; i++)
		similarity[i] = 0.0;
		
	for(long int i = 0; i < numCoins+1; i++)
	{
		Mat src_base, hsv_base;
		
		sprintf(buffer, "%ld", i);
		strcpy(name, "CoinsImage");
		strcat(name,buffer);
		strcat(name, ".JPG");
		
		src_base = imread(name);
		
		memset(buffer, 0, 40);
		memset(name,0,100);
				
		/// Convert to HSV
		cvtColor( src_base, hsv_base, COLOR_BGR2HSV );
		
		/// Using 50 bins for hue and 60 for saturation
		int h_bins = 50; int s_bins = 60;
		int histSize[] = { h_bins, s_bins };

		// hue varies from 0 to 179, saturation from 0 to 255
		float h_ranges[] = { 0, 180 };
		float s_ranges[] = { 0, 256 };

		const float* ranges[] = { h_ranges, s_ranges };

		// Use the o-th and 1-st channels
		int channels[] = { 0, 1 };

		/// Histograms
		MatND hist_coin;
		
		/// Calculate the histograms for the HSV images
		calcHist( &hsv_base, 1, channels, Mat(), hist_coin, 2, histSize, ranges, true, false );
		normalize( hist_coin, hist_coin, 0, 1, NORM_MINMAX, -1, Mat() );
		
		
		printf("************************\n");
		for(int j = 0; j < 5; j++)
		{
			/// Apply the histogram comparison methods
			int compare_method = 0;
			double base_coin = compareHist( hist_coin, hist_base[j], 0 );
			if(similarity[i] < base_coin)
				similarity[i] = base_coin;
			printf( " Method [%d] Perfect, Base-Half, Base-Test(1) : %f\n", 0, base_coin);
		}
		printf("************************\n");
	}
	for(long int i = 0; i <= numCoins; i++)
		printf("%f\n", similarity[i]);
}

int main( int argc, char** argv )
{
	/// Load an image
	src = imread( argv[1] );
	
	baseHistogram();

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
	
	compareHistogram();

	/// Wait until user exit program by pressing a key
	waitKey(0);

	return 0;
}
