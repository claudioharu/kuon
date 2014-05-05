#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace cv;

/** function rgb2hex: store the bits of the three channels into a single number*/
unsigned long rgb2long(int r, int g, int b)
{   
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

/*
Calculates a table of 256 assignments with the given number of distinct values.

Values are taken at equal intervals from the ranges [0, 128) and [128, 256),
such that both 0 and 255 are always included in the range.
*/
cv::Mat lookupTable(int levels) {
    int factor = 256 / levels;
    cv::Mat table(1, 256, CV_8U);
    uchar *p = table.data;

    for(int i = 0; i < 128; ++i) {
        p[i] = factor * (i / factor);
    }

    for(int i = 128; i < 256; ++i) {
        p[i] = factor * (1 + (i / factor)) - 1;
    }

    return table;
}

/*
Truncates channel levels in the given image to the given number of
equally-spaced values.

Arguments:

image
    Input multi-channel image. The specific color space is not
    important, as long as all channels are encoded from 0 to 255.

levels
    The number of distinct values for the channels of the output
    image. Output values are drawn from the range [0, 255] from
    the extremes inwards, resulting in a nearly equally-spaced scale
    where the smallest and largest values are always 0 and 255.

Returns:

Multi-channel images with values truncated to the specified number of
distinct levels.
*/
cv::Mat colorReduce(const cv::Mat &image, int levels) {
    cv::Mat table = lookupTable(levels);

    std::vector<cv::Mat> c;
    cv::split(image, c);
    for (std::vector<cv::Mat>::iterator i = c.begin(), n = c.end(); i != n; ++i) {
        cv::Mat &channel = *i;
        cv::LUT(channel.clone(), table, channel);
    }

    cv::Mat reduced;
    cv::merge(c, reduced);
    return reduced;
}

/**Detection of 1R$**/
bool oneReal(unsigned long centerColor, unsigned long radiusColor, int radius)
{
	if (radius >= 143 && radius <= 154)
	{
		if(centerColor == 0)
			if (radiusColor > 0) return true;
			else return false;
	}
	else return false;
}

/**Detection of 0,50R$**/
bool halfReal(unsigned long centerColor, unsigned long radiusColor, int radius)
{
	if (radius >= 120 && radius <= 135)
	{
		if(centerColor == 0)
			if (radiusColor == 0) return true;
			else return false;
		else return false;
	}
	else return false;
}

/** function main */
int main(int argc, char** argv)
{
  Mat src, src_gray, src_hsv, src_olhando;

  /// Read the image
  src = imread( argv[1], 1 );

  if( !src.data )
    { return -1; }

  /// Reduce the noise so we avoid false circle detection
  medianBlur(src, src, 5);

  /// Convert it to gray
  cvtColor( src, src_gray, CV_BGR2GRAY );

  /// Convert it to hsv
  cvtColor( src, src_hsv, CV_BGR2HSV );

  /// Reduce the noise so we avoid false circle detection
  
  medianBlur(src_hsv, src_hsv, 51);
  //GaussianBlur( src_hsv, src_hsv, Size(9, 9), 2, 2 );
  src_hsv = colorReduce(src_hsv, 2);
  
  
  vector<Vec3f> circles;

  /// Apply the Hough Transform to find the circles
  HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, 220, 188, 101, 0, 500 );

  /// Draw the circles detected
  for( size_t i = 0; i < circles.size(); i++ )
  {
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      
      /// Center color information 
      //Vec3i bgrCenter = src.at<Vec3b>(center.y, center.x);
      
      /// Center color information hsv
      Vec3i bgrCenterHsv = src_hsv.at<Vec3b>(center.y, center.x);

      /// Store the rgb channels into a single number
      unsigned long bgrCenterHsv2long = rgb2long(bgrCenterHsv[0], bgrCenterHsv[1], bgrCenterHsv[2]);
      
	   std::cout << "rgb: "<< bgrCenterHsv2long << "	," << center.x << "," << center.y << "\n";
      //std::cout << "rgb: "<< bgrCenterHsv << "," << center.x << "," << center.y << "\n";
      
      /// Radius color information
      //Vec3b bgrRadius = src.at<Vec3b>(center.y, center.x+radius);
      
       /// Radius color information hsv
      Vec3b bgrRadiusHsv = src_hsv.at<Vec3b>(center.y, center.x+radius);
      
      /// Store the rgb channels into a single number
      unsigned long bgrRadiusHsv2long = rgb2long(bgrRadiusHsv[0], bgrRadiusHsv[1], bgrRadiusHsv[2]);
      
      std::cout << "rgb: "<< bgrRadiusHsv2long << "	," << center.x+radius << "," << center.y << " radius: "<< radius<<"\n\n";
      
      //printf( "sub %ul" , bgrCenterHsv2long - bgrRadiusHsv2long); printf("\n\n");
      std::cout << "Eh Um real:" << oneReal(bgrCenterHsv2long, bgrRadiusHsv2long, radius) <<"\n\n";
      //std::cout << bgrRadius << "," << center.x+radius << "," << center.y << " radius: "<< radius<<"\n\n";
      
      // circle center
      circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
      // circle outline
      circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
   }

  /// Show your results
  namedWindow( "Hough Circle Transform Demo", CV_WINDOW_NORMAL );
  imshow( "Hough Circle Transform Demo", src_hsv );

  waitKey(0);
  return 0;
}
