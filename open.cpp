#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

using namespace cv;

/// Global variables

Mat src, src_gray;
Mat dst, detected_edges;

int edgeThresh = 1;
int lowThreshold = 10;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

/**
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
void CannyThreshold(int, void*)
{
   /// Reduce the noise so we avoid false circle detection
   //medianBlur(src_gray, src_gray, 7);
   blur( src_gray, src_gray, Size(9,9) );

   //threshold( src_gray, dst, 170, 255,THRESH_BINARY );

  /// Canny detector
  Canny( src_gray, src_gray, lowThreshold, lowThreshold*ratio, kernel_size );

  /// Using Canny's output as a mask, we display our result
  dst = Scalar::all(0);

 // src.copyTo( src_gray, detected_edges);
  
  //cvtColor( src_gray, src_gray, CV_BGR2GRAY );
  
  vector<Vec3f> circles;
  
  HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src.rows/6, 100, 50, 95, 165 );
  
  /// Draw the circles detected
  for( size_t i = 0; i < circles.size(); i++ )
  {
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      
      // circle center
      circle( src, center, 3, Scalar(255,0,0), -1, 8, 0 );

      // circle outline
      circle( src, center, radius, Scalar(255,0,0), 3, 8, 0 );
   }
  
  imshow( window_name, src );
 }


/** @function main */
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
  namedWindow( window_name, CV_WINDOW_NORMAL );

  /// Show the image
  CannyThreshold(0, 0);

  /// Wait until user exit program by pressing a key
  waitKey(0);

  return 0;
  }
