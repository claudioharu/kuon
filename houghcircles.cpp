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


/**Detection of 1R$**/
bool oneReal(unsigned long centerColor, unsigned long radiusColor, int radius)
{
	if (radius >= 148 && radius <= 154)
	{
		unsigned long sub = centerColor - radiusColor;
		if (sub < 0) sub = -sub;
		if (sub > 100000) return true;
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
  //GaussianBlur( src_hsv, src_hsv, Size(9, 9), 2, 2 );
  medianBlur(src_hsv, src_hsv, 5);
  
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
      //std::cout << "hue: "<< hsvCenter[0] << "," << center.x << "," << center.y << "\n";
      
      /// Radius color information
      //Vec3b bgrRadius = src.at<Vec3b>(center.y, center.x+radius);
      
       /// Radius color information hsv
      Vec3b bgrRadiusHsv = src_hsv.at<Vec3b>(center.y, center.x+radius);
      
      /// Store the rgb channels into a single number
      unsigned long bgrRadiusHsv2long = rgb2long(bgrRadiusHsv[0], bgrRadiusHsv[1], bgrRadiusHsv[2]);
      
      std::cout << "rgb: "<< bgrRadiusHsv2long << "	," << center.x+radius << "," << center.y << " radius: "<< radius<<"\n\n";
      
      std::cout << "Eh Um real:" << oneReal(bgrCenterHsv2long, bgrRadiusHsv2long, radius) <<"\n\n";
      //std::cout << bgrRadius << "," << center.x+radius << "," << center.y << " radius: "<< radius<<"\n\n";
      
      // circle center
      circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
      // circle outline
      circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
   }

  /// Show your results
  namedWindow( "Hough Circle Transform Demo", CV_WINDOW_NORMAL );
  imshow( "Hough Circle Transform Demo", src );

  waitKey(0);
  return 0;
}
