// name: Harrison Brookeman, Joseph Deguzman, Hong Moon, Zachary Zydron
// date: 2013-12-13
// version: 2.1

// Acknowledgement: Some of the code was brought and modified from the book "Mastering OpenCV with Practical Computer Vision Projects" by David Millan Escriva

/******************************************************************************
*   Ch5 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

#ifndef DetectRegions_h
#define DetectRegions_h

#include <string.h>
#include <vector>

#include "Plate.h"

#include <cv.h>
#include <highgui.h>
#include <cvaux.h>

using namespace std;
using namespace cv;

class DetectRegions{
    public:
        DetectRegions();
        string filename;
        void setFilename(string f);
        bool saveRegions;
        bool showSteps;
	int plate_width; // *****
	int plate_height; // *****
        vector<Plate> run(Mat input);
    private:
        vector<Plate> segment(Mat input);
        bool verifySizes(RotatedRect mr);
        Mat histeq(Mat in);
};

#endif
