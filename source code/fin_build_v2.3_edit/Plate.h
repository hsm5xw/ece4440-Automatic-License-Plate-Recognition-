// name: Harrison Brookeman, Joseph Deguzman, Hong Moon, Zachary Zydron
// date: 2013-12-13
// version: 2.1

// Acknowledgement: Some of the code was brought and modified from the book "Mastering OpenCV with Practical Computer Vision Projects" by David Millan Escriva

/******************************************************************************
*   Ch5 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

#ifndef Plate_h
#define Plate_h

#include <string.h>
#include <vector>

#include <cv.h>
#include <highgui.h>
#include <cvaux.h>

using namespace std;
using namespace cv;

class Plate{
    public:
        Plate();
        Plate(Mat img, Rect pos);
        string str();
	void set_ocr_position(Rect pos); // ******

        Rect position;
	Rect ocr_position; // *********
        Mat plateImg;
        vector<char> chars;
        vector<Rect> charsPos;        
};

#endif
