#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <time.h>

#define Max 9000
#define UMREAL 0
#define DEZCENT 1
#define CINCOCENT 2
#define CINQCENT 3
#define VINTCINCOCENT 4

using namespace cv;

/// Global variables
Mat src, src_gray;
Mat dst;
MatND hist_base[6];
int baseRadius[6];
int Radius[100];
long int numCoins;

void cropCoins(Point center, int radius, long int number){

	char buffer[40];
	char name[100];
	cv::Mat eyeImg;

	int leftROI = center.x - radius;
	int topROI = center.y - radius;

	int width, height;
	width = height = 2*radius;
	
	if(leftROI < 0) 
		leftROI = 0;
	if(leftROI+width > src.cols) 
		width -= leftROI+width - src.cols;
	if(topROI < 0) 
		topROI = 0;
	if(topROI+height > src.rows) 
		height -= topROI+height - src.rows;

	eyeImg = src(cv::Rect(	leftROI,	// ROI x-offset, left coordinate
				topROI,		// ROI y-offset, top coordinate 
				width,		// ROI width
				height));	// ROI height

	sprintf(buffer, "%ld", number);
	strcpy(name, "CoinsImage");
	strcat(name,buffer);
	strcat(name, ".JPG");
			
	imwrite( name, eyeImg );
}

void circleDetection(int, void*)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	Mat circleImage;

	//Binarization
	threshold( src_gray, src_gray, 210, 255, THRESH_BINARY_INV );
	imwrite("coinsThreshold.png", src_gray);

        //Blur the image so we can avoid noise
	blur( src_gray, src_gray, Size(3,3) );
	imwrite("coinsBlur.png", src_gray);

        //Close Operation
        morphologyEx(src_gray, src_gray, MORPH_CLOSE , getStructuringElement(MORPH_RECT, Size(3,3), Point(-1,-1)),Point(-1,-1), 9);
        imwrite("coinsClose.png", src_gray);

        //Blur the image so we can avoid noise
	blur( src_gray, src_gray, Size(3,3) );
	imwrite("coinsBlur2.png", src_gray);

        //Binarization again
	threshold( src_gray, src_gray, 120, 255, THRESH_BINARY_INV );
	imwrite("coinsThreshold2.png", src_gray);

        // Canny detector
        Canny( src_gray, src_gray, 10, 30, 3 );
        imwrite("coinsCanny.png", src_gray);

        //Dilate a little
        dilate(src_gray, src_gray,  getStructuringElement(MORPH_RECT, Size(2, 2), Point(-1, -1) ), Point(-1, -1), 2 );

        //Blur the image so we can avoid noise
	blur( src_gray, src_gray, Size(9,9) );
	imwrite("coinsBlur3.png", src_gray);

        //Detect circles
	vector<Vec3f> circles;
	HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, 220, 30, 30, 100, 145 );

	//Draw the circles detected
	for( size_t i = 0; i < circles.size(); i++ )
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		Radius[i] = cvRound(circles[i][2]);

		//Crop image according to the drawn circles
		cropCoins(center, radius, i);

		numCoins = i;
		// circle center
		circle( dst, center, 3, Scalar(255,0,0), -1, 8, 0 );

		// circle outline
		circle( dst, center, radius, Scalar(255,0,0), 3, 8, 0 );
	}
	imwrite("coins.png", dst);
}

double printAnswer(int bestColor, int bestSize)
{
        double coin = 0.0;
        printf("************************\n");
	switch (bestColor)
	{	
		case UMREAL:
                        if(bestSize == DEZCENT){
		                printf("Coin: %d\n", 10);
		                coin += 0.10;
                        }
                        else{
                                printf("Coin: %d\n", 1);
			        coin += 1.0;
                        }
			break;
		case DEZCENT:
                        if(bestSize != VINTCINCOCENT){
                                if(bestSize != UMREAL){
			                printf("Coin: %d\n", 10);
			                coin += 0.10;
                                }
                                else{
                                        printf("Coin: %d\n", 1);
			                coin += 1.0;
                                }
                        }
                        else{
                                printf("Coin: %d\n", 25);
			        coin += 0.25;
                        }
			break;
		case CINCOCENT:
		        printf("Coin: %d\n", 5);
		        coin += 0.05;
			break;
		case CINQCENT:
		        printf("Coin: %d\n", 50);
		        coin += 0.50;
			break;
		case VINTCINCOCENT:
			if(bestSize != VINTCINCOCENT){
                                if(bestSize != UMREAL){
			                printf("Coin: %d\n", 10);
			                coin += 0.10;
                                }
                                else{
                                        printf("Coin: %d\n", 1);
			                coin += 1.0;
                                }
                        }
                        else{
                                printf("Coin: %d\n", 25);
			        coin += 0.25;
                        }
			break;
	}
        return coin;
}

void baseHistogram()
{
	Mat base_coin, hsv_base_coin;
	char name[100];
	char buffer[40];
	
	for(int i = 0; i < 5; i++)
	{
		sprintf(buffer, "%d", i);
		strcpy(name, "Coin");
		strcat(name,buffer);
		strcat(name, ".JPG");
		
		base_coin =imread( name );
		
		memset(buffer, 0, 40);
		memset(name,0,100);
				
		baseRadius[i] = base_coin.cols;

		/// Convert to HSV
		cvtColor( base_coin, hsv_base_coin, COLOR_BGR2HSV );

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
		calcHist( &hsv_base_coin, 1, channels, Mat(), hist_base[i], 2, histSize, ranges, true, false );
		normalize( hist_base[i], hist_base[i], 0, 1, NORM_MINMAX, -1, Mat() );
	}
}

void compareAndPrint()
{
	char buffer[40];
	char name[100];
	double similarity[numCoins+1];
        double coins = 0.0;
	
	for(long int i = 0; i <numCoins+1; i++)
		similarity[i] = 0.0;

        printf("\n\n");
		
	for(long int i = 0; i < numCoins+1; i++)
	{
		Mat tst_coin, hsv_tst_coin;
		
		sprintf(buffer, "%ld", i);
		strcpy(name, "CoinsImage");
		strcat(name,buffer);
		strcat(name, ".JPG");
		
		tst_coin = imread(name);
		
		memset(buffer, 0, 40);
		memset(name,0,100);
				
		// Convert to HSV
		cvtColor( tst_coin, hsv_tst_coin, COLOR_BGR2HSV );
		
		// Using 50 bins for hue and 60 for saturation
		int h_bins = 50; int s_bins = 60;
		int histSize[] = { h_bins, s_bins };

		// hue varies from 0 to 179, saturation from 0 to 255
		float h_ranges[] = { 0, 180 };
		float s_ranges[] = { 0, 256 };

		const float* ranges[] = { h_ranges, s_ranges };

		// Use the o-th and 1-st channels
		int channels[] = { 0, 1 };

		// Histogram
		MatND hist_coin;
		
		// Calculate the histograms for the HSV images
		calcHist( &hsv_tst_coin, 1, channels, Mat(), hist_coin, 2, histSize, ranges, true, false );
		normalize( hist_coin, hist_coin, 0, 1, NORM_MINMAX, -1, Mat() );
		
		int bestColor, bestSize;

                // Compare sizes
		int sub = Max;
		for (int j = 0; j < 5; j++)
		{
			int aux = abs(baseRadius[j] - tst_coin.cols);
			if(sub >= aux)
			{
				sub = aux;
				bestSize = j;
			}
		}

                // Apply the histogram comparison methods
		for(int j = 0; j < 5; j++)
		{
			int compare_method = 0;
			double base_coin = compareHist( hist_coin, hist_base[j], 0 );
			if(similarity[i] < base_coin)
			{
				similarity[i] = base_coin;
				bestColor = j;
			}
		}
                coins += printAnswer( bestColor, bestSize);
        }

        printf("************************\n\n");
        printf("Total: R$%.2f\n\n\n", coins);
}

int main( int argc, char** argv )
{
	/// Load an image
	dst = imread( argv[1] );

        // Sharpening
        GaussianBlur(dst, src, Size(0, 0), 5);
        addWeighted(dst, 2.5, src, -1.5, 0, src);
        imwrite("coinsSharpening.png", src);

        //Change saturation
        // BGR to HSV
        cvtColor(src, src, CV_BGR2HSV);    

        vector<Mat> hsv(3);
        split(src, hsv);
        hsv[1].convertTo(hsv[1], -1, 1, 20);
        merge(hsv, src);

        // HSV to BGR
        cvtColor(src, src, CV_HSV2BGR);
        imwrite("coinsSaturated.png", src);

        //Change brightness and contrast
        for( int y = 0; y < src.rows; y++ )
		{ 
			for( int x = 0; x < src.cols; x++ )
			{ 
				for( int c = 0; c < 3; c++ )
				{
					src.at<Vec3b>(y,x)[c] =
					saturate_cast<uchar>( 2.0*( src.at<Vec3b>(y,x)[c] ) + 70 );
				}
			}
		}
        imwrite("coinsBrightness.png", src);
	
	srand (time(NULL));
	
	baseHistogram();

	if( !src.data )
	{ return -1; }

	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );
	//imwrite("coinsGrayScale.png", src_gray);

	circleDetection(0, 0);
	
	compareAndPrint();

	
	return 0;
}
