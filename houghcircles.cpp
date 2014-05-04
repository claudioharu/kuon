#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace cv;


/** @function main */
int main(int argc, char** argv)
{
  Mat src, src_gray, src_hsv;

  /// Read the image
  src = imread( argv[1], 1 );

  if( !src.data )
    { return -1; }

  medianBlur(src, src, 5);

  /// Convert it to gray
  cvtColor( src, src_gray, CV_BGR2GRAY );
  cvtColor( src, src_hsv, CV_BGR2HSV );

  /// Reduce the noise so we avoid false circle detection
  //GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2 );

  vector<Vec3f> circles;

  /// Apply the Hough Transform to find the circles
  HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, 220, 188, 101, 0, 500 );

  /// Draw the circles detected
  for( size_t i = 0; i < circles.size(); i++ )
  {
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      
      
      ///center color information
      Vec3b bgrCenter = src.at<Vec3b>(center.y, center.x);
      std::cout << bgrCenter << "," << center.x << "," << center.y << "\n";
      
      ///radius color information
      Vec3b bgrRadius = src.at<Vec3b>(center.y, center.x+radius);
      std::cout << bgrRadius << "," << center.x+radius << "," << center.y << " radius: "<< radius<<"\n\n";
      
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
