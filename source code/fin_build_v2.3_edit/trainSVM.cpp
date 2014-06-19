// name: Harrison Brookeman, Joseph Deguzman, Hong Moon, Zachary Zydron
// date: 2013-12-13
// version: 2.1

// Acknowledgement: Some of the code was brought and modified from the book "Mastering OpenCV with Practical Computer Vision Projects" by David Millan Escriva

/******************************************************************************
*   Ch5 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

// Main entry code OpenCV

#include <cv.h>
#include <highgui.h>
#include <cvaux.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace cv;


Mat histeq(Mat in)
{
    Mat out(in.size(), in.type());
    if(in.channels()==3){
        Mat hsv;
        vector<Mat> hsvSplit;
        cvtColor(in, hsv, CV_BGR2HSV);
        split(hsv, hsvSplit);
        equalizeHist(hsvSplit[2], hsvSplit[2]);
        merge(hsvSplit, hsv);
        cvtColor(hsv, out, CV_HSV2BGR);
    }else if(in.channels()==1){
        equalizeHist(in, out);
    }

    return out;

}



int main ( int argc, char** argv )
{
    cout << "OpenCV Training SVM Automatic Number Plate Recognition\n";
    cout << "\n";

    char* path_Plates;
    char* path_NoPlates;
    int numPlates;
    int numNoPlates;
    int imageWidth=171;
    int imageHeight=85;

    //Check if user specify image to process
    if(argc >= 5 )
    {
        numPlates= atoi(argv[1]);
        numNoPlates= atoi(argv[2]);
        path_Plates= argv[3];
        path_NoPlates= argv[4];

    }else{
        cout << "Usage:\n" << argv[0] << " <num Plate Files> <num Non Plate Files> <path to plate folder files> <path to non plate files> \n";
        return 0;
    }        

    Mat classes;//(numPlates+numNoPlates, 1, CV_32FC1);
    Mat trainingData;//(numPlates+numNoPlates, imageWidth*imageHeight, CV_32FC1 );

    Mat trainingImages;
    vector<int> trainingLabels;

    for(int i=0; i< numPlates; i++)
    {

        stringstream ss(stringstream::in | stringstream::out);
        ss << path_Plates << i << ".jpg";
        //Mat img=imread(ss.str(), 0);

	// ***************************
	Mat img=imread(ss.str(), 1); 
	Mat img_gray;
	cvtColor(img, img_gray, CV_BGR2GRAY);
	blur(img_gray,img_gray, Size(3,3));
	img_gray = histeq(img_gray);

	img_gray = img_gray.reshape(1, 1);
	img_gray.convertTo(img, CV_32FC1);
	// ****************************

	if(i==0)
	{
		cout << "plate_region rows: " << img_gray.rows << endl;   // ***********
		cout << "plate_region cols: " << img_gray.cols << endl << endl; // ***************
	}
        trainingImages.push_back(img_gray); // *************
        trainingLabels.push_back(1);
    }

    for(int i=0; i< numNoPlates; i++)
    {
        stringstream ss(stringstream::in | stringstream::out);
        ss << path_NoPlates << i << ".jpg";
        //Mat img=imread(ss.str(), 0);

	// ***************************
	Mat img=imread(ss.str(), 1); 
	Mat img_gray;
	cvtColor(img, img_gray, CV_BGR2GRAY);
	blur(img_gray,img_gray, Size(3,3));
	img_gray = histeq(img_gray);

	img_gray = img_gray.reshape(1, 1);
	img_gray.convertTo(img, CV_32FC1);
	// ****************************

	if(i==0)
	{
		cout << "non_plate_region rows: " << img_gray.rows << endl; // ******
		cout << "non_plate_region cols: " << img_gray.cols << endl; // ******
	}
        trainingImages.push_back(img_gray);  // **************
        trainingLabels.push_back(0);

    }
    // ***** Debugging messages ***********

    cout << "trainingLabels: " << trainingLabels.at(0) << endl;

    // **************************************

    Mat(trainingImages).copyTo(trainingData);
    //trainingData = trainingData.reshape(1,trainingData.rows);
    trainingData.convertTo(trainingData, CV_32FC1);
    Mat(trainingLabels).copyTo(classes);

    FileStorage fs("SVM.xml", FileStorage::WRITE);
    fs << "TrainingData" << trainingData;
    fs << "classes" << classes;
    fs.release();

    return 0;
}
