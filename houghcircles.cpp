#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace cv;

/*
Store the bits of the three channels into a single number
*/
unsigned long rgb2long(int r, int g, int b)
{   
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

/*
Calculates a table of 256 assignments with the given number of distinct levels.
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
Truncates channel levels in the given image to the specified number of
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

/**Detection of R$1,00**/
bool detect1Real(unsigned long centerColor, unsigned long radiusColor, int radius)
{
	if(centerColor == 0)
		if (radiusColor > 0) return true;
		else return false;
}

/**Detection of R$0,50**/
bool detect50Cent(unsigned long centerColor, unsigned long radiusColor, int radius)
{
	if(centerColor == 0)
		if (radiusColor == 0) return true;
		else return false;
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

  /// Blur the image so we can have a homogeneous color
  medianBlur(src_hsv, src_hsv, 51);

  /// Reduce the number of colors in an image
  src_hsv = colorReduce(src_hsv, 2);
    
  
  /// Canny detector
  Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

  
  vector<Vec3f> circles;

  /// Apply the Hough Transform to find the circles
  HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, 220, 188, 101, 0, 500 );

  /// Draw the circles detected
  for( size_t i = 0; i < circles.size(); i++ )
  {
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      
      /// Center color information
      Vec3i bgrCenterHsv = src_hsv.at<Vec3b>(center.y, center.x);

      /// Store the rgb channels into a single number
      unsigned long bgrCenterHsv2long = rgb2long(bgrCenterHsv[0], bgrCenterHsv[1], bgrCenterHsv[2]);
      
      /// Radius color information
      Vec3b bgrRadiusHsv = src_hsv.at<Vec3b>(center.y, center.x+radius);
      
      /// Store the rgb channels into a single number
      unsigned long bgrRadiusHsv2long = rgb2long(bgrRadiusHsv[0], bgrRadiusHsv[1], bgrRadiusHsv[2]);
     
      std::cout << "Eh Um real? " << detect1Real(bgrCenterHsv2long, bgrRadiusHsv2long, radius) <<"\n";
      std::cout << "Eh 50 centavos? " << detect50Cent(bgrCenterHsv2long, bgrRadiusHsv2long, radius-1) <<"\n\n";
      
      // circle center
      circle( src_hsv, center, 3, Scalar(255,0,0), -1, 8, 0 );

      // circle outline
      circle( src_hsv, center, radius, Scalar(255,0,0), 3, 8, 0 );
   }

  /// Show your results
  namedWindow( "Hough Circle Transform Demo", CV_WINDOW_NORMAL );
  imshow( "Hough Circle Transform Demo", src_hsv );

  waitKey(0);
  return 0;
}
