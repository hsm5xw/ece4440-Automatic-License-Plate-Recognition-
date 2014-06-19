
// Main File

// name: Harrison Brookeman, Joseph Deguzman, Hong Moon, Zachary Zydron
// date: 2013-12-13
// version: 2.1

// Acknowledgement: Some of the code was brought and modified from the book "Mastering OpenCV with Practical Computer Vision Projects" by David Millán Escrivá

/******************************************************************************
*   Ch5 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <sqlite3.h> 

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <stdio.h>			
#include <stdlib.h>			
#include <fstream>			

#include <cv.h>				
#include <highgui.h>		
#include <cvaux.h>			
#include <ml.h>	

#include <sys/stat.h>	
#include <sys/types.h>
#include <unistd.h>		

#include <iostream>			
#include <vector>				
#include <string>			

#include "DBManager.h"
#include "DetectRegions.h"
//#include "OCR.h"

using namespace std;
using namespace cv;


bool fileExists(const char* filename) {
	
	struct stat fileInfo;
	return stat(filename, &fileInfo) == 0;
}

string getFilename(string s) {
		
	char sep = '/';
    	char sepExt='.';

   	#ifdef _WIN32
        sep = '\\';
    	#endif

    size_t i = s.rfind(sep, s.length( ));
    
	if (i != string::npos) 
	{
       		string fn= (s.substr(i+1, s.length( ) - i));
        	size_t j = fn.rfind(sepExt, fn.length( ));
        
		if (i != string::npos)
            		return fn.substr(0,j);
        	else 
            		return fn;
        }
	else 
	{
        	return "capture.jpg";
    	}
}

string get_plate_regions(const char* imageName) {
	Mat input_image;
	input_image=imread(imageName,1);
	
	string filename_withoutExt=getFilename(imageName);
 
	//cout << "working with file: " << filename_withoutExt << "\n";
	
	// Detect Possible Plate Regions
	DetectRegions detectRegions;
	detectRegions.setFilename(filename_withoutExt);
	detectRegions.saveRegions=false;
	detectRegions.showSteps=false;
	vector<Plate> possible_regions = detectRegions.run(input_image); 				// Adding the possible regions to the vector
	// 
	
	//if (possible_regions.size() != 0) {
	//	cout << "Preprocessing rows: " << possible_regions[0].plateImg.rows << endl;
	//	cout << "Preprocessing cols: " << possible_regions[0].plateImg.cols << endl;
	//}
	
	cout << "\nNum of possible plates: " << possible_regions.size()<< endl << endl; // *******************************************

        // Skip the SVM if only one possible plate has been detected
	if( possible_regions.size() > 1)
	{	
		//SVM for each plate region to get valid car plates
    		//Read file storage.
        
    		FileStorage fs;
   		fs.open("SVM.xml", FileStorage::READ);
    		Mat SVM_TrainingData;
    		Mat SVM_Classes;
    		fs["TrainingData"] >> SVM_TrainingData;
    		fs["classes"] >> SVM_Classes;
    
		//Set SVM params
    		CvSVMParams SVM_params;
    		SVM_params.svm_type = CvSVM::C_SVC;
    		SVM_params.kernel_type = CvSVM::LINEAR; //CvSVM::LINEAR;
    		SVM_params.degree = 0;
    		SVM_params.gamma = 1;
    		SVM_params.coef0 = 0;
    		SVM_params.C = 1;
    		SVM_params.nu = 0;
   		 SVM_params.p = 0;
    		SVM_params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 0.01);
    
		//Train SVM
    		CvSVM svmClassifier(SVM_TrainingData, SVM_Classes, Mat(), Mat(), SVM_params);

    	
    		//For each possible plate, classify with svm if it's a plate or not
    		vector<Plate> plates;
   	 	for(int i=0; i< possible_regions.size(); i++)
   	 	{
     	   		Mat img=possible_regions[i].plateImg;
     	   		Mat p= img.reshape(1, 1);

           		p.convertTo(p, CV_32FC1);

	  	 	//cout << "rows: " << p.rows << endl;
			//cout << "cols: " << p.cols << endl;

        		int response = (int)svmClassifier.predict( p );
       		 	if(response==1)
            		plates.push_back(possible_regions[i]);
    		}

    		//cout << "Num plates detected: " << plates.size() << "\n";
	}

	//Iterating through the plate regions
	int counter = 0;
    	string fileNameTemplate = "temp_output_image";
   	string firstChunk = "temp_output/";
    	string lastChunk = ".jpg";

	for (int i = 0; i < possible_regions.size(); i++) {
		Plate plate = possible_regions[i];
		
		double aspectRatio = 0;
		double pos_x = possible_regions[i].position.x;
		double pos_y = possible_regions[i].position.y;
		double pos_width = possible_regions[i].position.width;
		double pos_height = possible_regions[i].position.height;
		
		if ((pos_width <= 0) || (pos_height <= 0) || (pos_x <= 0) || (pos_y <= 0)) continue;
		if (pos_height != 0) aspectRatio = pos_width / pos_height;
		
		// Outputing individual region information 
		//cout << "Possible plate " << i << "'s Rect x: " << pos_x << endl; 			//        
		//cout << "Possible plate " << i << "'s Rect y: " << pos_y << endl; 			// ********
		//cout << "Possible plate " << i << "'s Rect width: " << pos_width << endl; 		// ********
		//cout << "Possible plate " << i << "'s Rect height: " << pos_height << endl; 		// *******
		//cout << "Possible plate " << i << "'s Rect aspect ratio: " << aspectRatio << endl; 	// *******	
		
		rectangle(input_image, plate.position, Scalar(0,0,200));
		
		// Locating the License Plate Text (Only)
		int ocrpos_x = (int) (pos_x + pos_width * 0.05);					// Bring in width by 5% on the right
		int ocrpos_y = (int) (pos_y + pos_height * 0.260);					// Bring down height 26% from the top
		float ocrpos_width = pos_width * 0.9;							// Bring in width by 5% on the left
		float ocrpos_height = pos_height * 0.55;						// Bring up height by 19% from the bottom
		
		float ocrpos_center_x = ocrpos_x + ocrpos_width * 0.5;
		float ocrpos_center_y = ocrpos_y + ocrpos_height * 0.5;
		
		Rect ocrpos_rect = Rect(ocrpos_x, ocrpos_y, ocrpos_width, ocrpos_height);		// Drawing new Rectange around cropped license plate text
		
		if (((ocrpos_x + ocrpos_width) >= input_image.cols) || ((ocrpos_y + ocrpos_height) >= input_image.rows)) {
			cout << "Skip image " << i << endl << endl;
			continue;
		}
		
		Mat ocr_img;
		getRectSubPix(possible_regions[i].plateImg, Size(ocrpos_width, ocrpos_height), Point2f(ocrpos_center_x, ocrpos_center_y), ocr_img);
		
		possible_regions[i].set_ocr_position(ocrpos_rect);
		
		input_image(ocrpos_rect).copyTo(ocr_img);

 		rectangle(input_image, possible_regions[i].ocr_position, Scalar(0,255,0));
		
		char numstr[100]; 									// enough to hold all numbers up to 64-bits
		sprintf(numstr, "%d", counter);
		string result = fileNameTemplate + numstr;

		string path = firstChunk + result + lastChunk;

		imwrite(path,ocr_img); //Save cropped image to desired path		
		counter += 1; 										// increment to next plate in vector
	}
	
	//Displaying Analyzed Input-Image
	
	if( possible_regions.size() > 0)
	{
		namedWindow("Plate Detected", CV_WINDOW_NORMAL);	
		imshow("Plate Detected", input_image);

        	waitKey(30);
	}

	string fin = "temp_output/temp_output_image0.jpg";
	return fin;
}

bool isInvalidCharacter(char c) {
	
	switch(c) {
		case '\'':
		case '\"':
		case '{':
		case '}':
		case '[':
		case ']':
		case '*':
		case ' ':
			return true;
		default:
			return false;
	}
}


string getTessOutput(string s)
{
	string license_number = "";
 
	const int MAX_ATTEMPT = 5;
	int attempt_count = 0;

  	
	while( license_number.length() != 8 )
	{
		if(attempt_count > MAX_ATTEMPT)
		{
			license_number = "";
			break;
		}

		int ret = system(s.c_str());
	
		if(ret == -1) 
		{
			cout << "Error occurred while running the tesserect" << endl;
			return "error";
   		}
		
		if(attempt_count != 0 ) 
			cout << "Attempting to detect the license plate ... " << attempt_count << endl;

		license_number = "";
		ifstream infile;
   		infile.open("out.txt");

		getline(infile, license_number); 		
		license_number.erase(remove_if(license_number.begin(), license_number.end(), &isInvalidCharacter), license_number.end());

		string dash = "-";
		int dash_loc = license_number.find(dash);
		if (dash_loc > 3) license_number.erase(3, dash_loc-3);

		int size = license_number.size();
		if (size > 8 && license_number.at(size-1)=='1') license_number.erase(size-1, 1);

		if(dash_loc == std::string::npos)
		{
			license_number.replace(3, 1, "-");	// add a dash if a dash is not recognized
		}

		// Character filters: Given that only digits can appeaer after the dash, replace incorrect characters to likely character candidates
		for(int character_loc = 4; character_loc < 8; character_loc++)
		{
			char c = license_number.at(character_loc);

			switch(c) {
				case 'S': license_number.replace(character_loc, 1, "5"); break;
				case 'K': license_number.replace(character_loc, 1, "8"); break;	
				case 'I': license_number.replace(character_loc, 1, "1"); break;
				case 'O': license_number.replace(character_loc, 1, "0"); break;
			
				default:  break;
			}
		}

		// cout << "License number size: " << license_number.length() << endl;
    
   		infile.close();

		attempt_count++;
	}

		

	return license_number;
}



string run_tess(string imageName) {
	if(fileExists("out.txt")) system("rm out.txt");	
	
	string s = "tesseract " + imageName + " out letters";
	
	int back_fd, new_fd;					//Code added to redirect "Tesseract ..." Message
	fflush(stdout);						//Code added to redirect "Tesseract ..." Message
	back_fd = dup(2);					//Code added to redirect "Tesseract ..." Message	
	new_fd = open("/dev/null", O_WRONLY);			//Code added to redirect "Tesseract ..." Message
	dup2(new_fd,2);						//Redirecting stdout to /dev/null
	close(new_fd);						//Code added to redirect "Tesseract ..." Message

	string license_number = getTessOutput(s);		//Run Tesseract
	
	fflush(stdout);						//Code added to redirect "Tesseract ..." Message
	dup2(back_fd,2);					//Code added to redirect "Tesseract ..." Message
	close(back_fd);						//Code added to redirect "Tesseract ..." Message

		
	// End of Operation Additions
	
	if (fileExists(imageName.c_str())) {
		s = "rm " + imageName;		
		system(s.c_str());
	}

	string capture = "capture.jpg";
	if (fileExists(capture.c_str())) {
		s = "rm capture.jpg";		
		system(s.c_str());
	}

	return license_number;
}

Mat histeq(Mat in) {
    
	Mat out(in.size(), in.type());
	if(in.channels()==3) {
        Mat hsv;
        vector<Mat> hsvSplit;
        cvtColor(in, hsv, CV_BGR2HSV);
        split(hsv, hsvSplit);
        equalizeHist(hsvSplit[2], hsvSplit[2]);
        merge(hsvSplit, hsv);
        cvtColor(hsv, out, CV_HSV2BGR);
    } else if (in.channels()==1) {
        equalizeHist(in, out);
    }

    return out;
}

bool trainSVM(int numPlates, int numNoPlates, string pr_path, string npr_path, int imageWidth, int imageHeight) {
	char* path_Plates = new char[pr_path.length() + 1];
	strcpy(path_Plates,pr_path.c_str());
	char* path_NoPlates = new char[npr_path.length() + 1];
	strcpy(path_NoPlates,npr_path.c_str());
	
	Mat classes;						//(numPlates+numNoPlates, 1, CV_32FC1);
    	Mat trainingData;					//(numPlates+numNoPlates, imageWidth*imageHeight, CV_32FC1 );

    	Mat trainingImages;
    	vector<int> trainingLabels;
	
	for (int i = 0; i < numPlates; i++) {
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
		
		if(i==0) {
			cout << "plate_region rows: " << img_gray.rows << endl;   // ***********
			cout << "plate_region cols: " << img_gray.cols << endl << endl; // ***************
		}
        
		trainingImages.push_back(img_gray); // *************
        	trainingLabels.push_back(1);
    }

   for(int i=0; i< numNoPlates; i++) {
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

		if(i==0) {
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
	
	return true;	
}

bool gpio_get_value(unsigned int gpio)
{
        int fd;
        char buf[64];
        char ch;
	bool out=false;

        snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);

        fd = open(buf, O_RDONLY);
        if (fd < 0) {
                perror("gpio/get-value");
                return fd;
        }

        read(fd, &ch, 1);

        if (ch == '0') {
                out=false;
        } else {
                out=true;
        }

        close(fd);
        return out;
}



void prepareButtons() {
	
	if(!fileExists("/sys/class/gpio/gpio67/value")) {			
		//cout << "GPIO67 was not detected, creating..." << endl;
		system("echo 67 > /sys/class/gpio/export");
		system("echo in > /sys/class/gpio/gpio67/direction");
	}
	
	if(!fileExists("/sys/class/gpio/gpio68/value")) {
		//cout << "GPIO68 was not detected, creating..." << endl;
		system("echo 68 > /sys/class/gpio/export");
		system("echo in > /sys/class/gpio/gpio68/direction");
	}

	if(!fileExists("/sys/class/gpio/gpio44/value")) {
		//cout << "GPIO44 was not detected, creating..." << endl;
		system("echo 44 > /sys/class/gpio/export");
		system("echo in > /sys/class/gpio/gpio44/direction");
	}
	
	if(!fileExists("/sys/class/gpio/gpio26/value")) {
		//cout << "GPIO26 was not detected, creating..." << endl;
		system("echo 26 > /sys/class/gpio/export");
		system("echo in > /sys/class/gpio/gpio26/direction");
	}

	if(!fileExists("/sys/class/gpio/gpio46/value")) {
		//cout << "GPIO46 was not detected, creating..." << endl;
		system("echo 46 > /sys/class/gpio/export");
		system("echo in > /sys/class/gpio/gpio46/direction");
	}
	
	if(!fileExists("/sys/class/gpio/gpio65/value")) {
		//cout << "GPIO65 was not detected, creating..." << endl;
		system("echo 65 > /sys/class/gpio/export");
		system("echo in > /sys/class/gpio/gpio65/direction");
	}

	//if(fileExists("/sys/class/gpio/gpio67/value")) cout << "GPIO67 exists" << endl;			
	//if(fileExists("/sys/class/gpio/gpio68/value")) cout << "GPIO68 exists" << endl;		
	//if(fileExists("/sys/class/gpio/gpio44/value")) cout << "GPIO44 exists" << endl;			
	//if(fileExists("/sys/class/gpio/gpio26/value")) cout << "GPIO26 exists" << endl;
	//if(fileExists("/sys/class/gpio/gpio46/value")) cout << "GPIO46 exists" << endl;			
	//if(fileExists("/sys/class/gpio/gpio65/value")) cout << "GPIO65 exists" << endl;
}

void prepareDB() {
	
	if(!fileExists("test.db")) {
		//cout << "Database was not detected, creating..." << endl;
		system("./create-table");
	}
	//if(fileExists("test.db")) cout << "Database exists" << endl;
}

void prepareSVM() {
	
	if(!fileExists("SVM.xml")) {
		cout << "SVM.xml was not detected, creating..." << endl;
		system("./trainSVM 23 50 train/plate_regions/ train/non_plate_regions/");
	}
	if(fileExists("SVM.xml")) cout << "SVM.xml exists" << endl;
}


string captureImage() {
    
	int back_fd, new_fd;			//Code added to remove "INVALID .." terminal output
	fflush(stdout);				//Code added to remove "INVALID .." terminal output
	back_fd = dup(2);			//Code added to remove "INVALID .." terminal output
	new_fd = open("/dev/null", O_WRONLY);	//Redirect output to dev/null
	dup2(new_fd,2);				//Code added to remove "INVALID .." terminal output	
	close(new_fd);				//Code added to remove "INVALID .." terminal output	

	VideoCapture cap(0); 			// open the default camera
	
	cap.set(CV_CAP_PROP_FRAME_WIDTH,960);
    	cap.set(CV_CAP_PROP_FRAME_HEIGHT,720);

	if(!cap.isOpened()) return "error"; 	// check if camera is available
		
	Mat frame;
        cap >> frame; 				// get a new frame from camera
	imwrite("capture.jpg", frame);  
	
	fflush(stdout);				//Code added to remove "INVALID .." terminal output	
	dup2(back_fd,2);			//Redirect output to stdout		
	close(back_fd);				//Code added to remove "INVALID .." terminal output	
 
    	return "capture.jpg";
}

void demoOption_1(string image1, string image2, string image3) {
	prepareButtons();
	prepareDB();

	DBManager dbmanagerObj;

	//dbmanagerObj.insert("ASH-2345", "train/originals/3.jpg"); // ****** for testing
	//dbmanagerObj.insert("BSH-2345", "train/originals/4.jpg"); // ****** for testing
	//dbmanagerObj.insert("WSH-8345", "train/originals/5.jpg"); // ****** for testing 
	//dbmanagerObj.insert("ASH-2365", "train/originals/6.jpg"); // ****** for testing
	//dbmanagerObj.insert("ABC-1234", "train/originals/7.jpg"); // ****** for testing

	
	while (true) {
		bool push_67 = gpio_get_value(67);
		bool push_68 = gpio_get_value(68);
		bool push_44 = gpio_get_value(44);
		bool push_26 = gpio_get_value(26);
		bool push_46 = gpio_get_value(46);
		bool push_65 = gpio_get_value(65);
		

		if(push_67 || push_44 || push_46) {		//If a car is entering
			cout << "Vehicle Entering ... Please Wait ..." << endl;
			string captured;

			if(push_67) captured = "train/originals/" + image1 + ".jpg";
			if(push_44) captured = "train/originals/" + image2 + ".jpg";
			if(push_46) captured = "train/originals/" + image3 + ".jpg";

			push_67 = false;
			push_44 = false;
			push_46 = false;

			string cropped_image = get_plate_regions(captured.c_str());
			//cout << "Sending " << cropped_image << " to tesseract method" << endl;
			string license_number = run_tess(cropped_image);
			if (fileExists("temp_output/temp_output_image0.jpg")) 
				system("rm temp_output/temp_output_image0.jpg");
			
			destroyAllWindows();
			
			if(license_number.compare("") == 0) {
				cout << "\nYour license plate was not detected, please try again\n" << endl;
				continue;
			} else {
				dbmanagerObj.insert(license_number, captured);
				cout << "License Plate detected: " << license_number << endl;
				cout << "\n-------------------- Show result After Insert--------------- \n " << endl;
   				dbmanagerObj.select(); // show the result after the insert
   				cout << "\n-------------------- End ----------------------------------- \n" << endl;

				cout << "\nPlease press a button to enter or exit ... \n" << endl;
			}
		}

		if(push_68 || push_26 || push_65) {		//If a car is exiting
			cout << "Vehicle Exiting ... Please Wait ..." << endl;			
			string captured;

			if(push_68) captured = "train/originals/" + image1 + ".jpg";
			if(push_26) captured = "train/originals/" + image2 + ".jpg";
			if(push_65) captured = "train/originals/" + image3 + ".jpg";

			push_68 = false;
			push_26 = false;
			push_65 = false;

			string cropped_image = get_plate_regions(captured.c_str());
			//cout << "Sending " << cropped_image << " to tesseract method" << endl;
			
			string license_number = run_tess(cropped_image);
			if (fileExists("temp_output/temp_output_image0.jpg")) 
				system("rm temp_output/temp_output_image0.jpg");
			
			if(license_number.compare("") == 0) {
				cout << "\nYour license plate was not detected, please try again\n" << endl;
				continue;
			} else {

				cout << "License Plate detected: " << license_number << endl;
				double parking_rate = dbmanagerObj.calculate(license_number);

				if (parking_rate < 0)		// if the license plate is not found on the database 
				{
					string input = "";
					vector< vector<string> > wildCardResults = dbmanagerObj.selectWithWildCard(license_number);
										
					if( wildCardResults.empty() )
					{
						cout << "***** No searched result found ***** " << endl;
						destroyAllWindows();
						cout << "\nPlease press a button to enter or exit ... \n" << endl;
						continue;
					}

					//cout << "@@@@@ Show results from Wild Card Searches @@@@@" << endl;

					for( int i=0; i < wildCardResults.size(); i++)
					{
						string plateNumber(wildCardResults[i][0]);
						string imagePath = dbmanagerObj.getBaseImagePath() + plateNumber + ".jpg";

						//cout << "\nElement " << i << endl;
						cout << "plate_number = " << wildCardResults[i][0] << endl;
						cout << "admission_time = " << wildCardResults[i][1] << endl;
						cout << "admission_timestamp = " << wildCardResults[i][2] << endl;

						
						if( fileExists( imagePath.c_str() ) )
						{
							destroyAllWindows();						

							Mat input_image;
							input_image=imread(imagePath.c_str() ,1);

							if( input_image.size().width > 0 && input_image.size().height > 0)
							{
								namedWindow("Is this your car ?", CV_WINDOW_NORMAL);	
								imshow("Is this your car ?", input_image);
								waitKey(30);
							}	
						}	
						
						cout << "Is this your car ? [Yes, No ] > ";
						getline(cin, input);
						cout << "\nYou entered " << input << endl << endl;
						destroyAllWindows();
		
						if( input.compare("Yes") == 0 )
						{
							cout << "Found your car. Sorry for the inconvenience" << endl;

							// calculate the price again with the corrected license plate number	
							license_number = plateNumber;
							parking_rate = dbmanagerObj.calculate( license_number );	

							break;
						}
					}

					if( input.compare("No") == 0) // check the very last button input
					{
						cout << "***** We could not find your license plate from our database. We are very sorry " << endl;
						cout << "***** Your parking rate is our base price, $" << dbmanagerObj.getPaymentUnit() << endl << endl; // base parking rate
						destroyAllWindows();

						cout << "\nPlease press a button to enter or exit ... \n" << endl;
						continue; 
					}

				}

				dbmanagerObj.deleteRecord(license_number, captured);
				cout << "\n-------------------- Show result After Delete--------------- \n " << endl;
   				dbmanagerObj.select(); // show the result after the deletion
   				cout << "\n-------------------- End ----------------------------------- \n" << endl;
				destroyAllWindows();

				cout << "Please press a button to enter or exit ... \n" << endl;
			}
		}		
	}
}

void demoOption_2(string image1, string image2, string image3) {

	DBManager dbmanagerObj;

	while (true) {
		bool push_67 = gpio_get_value(67);			//Signifies a car wants to enter
		bool push_68 = gpio_get_value(68);			//Signifies a car wants to exit

		if (push_67) {
			push_67 = false;			
			cout << "Vehicle Entering ... Please Wait ..." << endl;
			string captured = captureImage();
			if (captured.compare("error") == 0) {
				cout << "camera was not detected try again" << endl;
				continue;
			}
			string cropped_image = get_plate_regions(captured.c_str());
			//cout << "Sending " << cropped_image << " to tesseract method" << endl;
			string license_number = run_tess(cropped_image);
			if (fileExists("temp_output/temp_output_image0.jpg")) 
				system("rm temp_output/temp_output_image0.jpg");
		
			if(license_number.compare("") == 0) {
				cout << "\nYour license plate was not detected, please try again\n" << endl;
				continue;
			} else {
				dbmanagerObj.insert(license_number, captured);
				cout << "License Plate detected: " << license_number << endl;
				cout << "\n-------------------- Show result After Insert--------------- \n " << endl;
   				dbmanagerObj.select(); // show the result after the insert
   				cout << "\n-------------------- End ----------------------------------- \n" << endl;

				cout << "************* Please press a button to enter or exit *********** \n" << endl;
			}
		}

		if (push_68) {
			push_68 = false;
			cout << "Vehicle Exiting ... Please Wait ..." << endl;			
			string captured = captureImage();
			if (captured.compare("error") == 0) {
				cout << "camera was not detected try again" << endl;
				continue;
			}

			string cropped_image = get_plate_regions(captured.c_str());
			//cout << "Sending " << cropped_image << " to tesseract method" << endl;
			string license_number = run_tess(cropped_image);
			if (fileExists("temp_output/temp_output_image0.jpg")) 
				system("rm temp_output/temp_output_image0.jpg");
			
			if(license_number.compare("") == 0) {
				cout << "\nYour license plate was not detected, please try again\n" << endl;
				continue;
			} else {

				//cout << "License Plate detected: " << license_number << endl;
				double parking_rate = dbmanagerObj.calculate(license_number);

				if (parking_rate < 0) 
					continue;

				dbmanagerObj.deleteRecord(license_number, captured);
				cout << "\n-------------------- Show result After Delete--------------- \n " << endl;
   				dbmanagerObj.select(); // show the result after the deletion
   				cout << "\n-------------------- End ----------------------------------- \n" << endl;

				cout << "************* Please press a button to enter or exit *********** \n" << endl;
			}
		}
	}
}

int main (int argc, char** argv) {

	prepareButtons();
	prepareDB();
	
	cout << "\n************* Welcome to the parking garage          *********** " << endl;
        cout << "************* Please press a button to enter or exit *********** \n" << endl;

	if(argc < 2 ) {
        	cout << "Usage:\n" << argv[0] << " <demo option> \n" << endl;
		cout << "<demo option> 1: Using pre-stored images" << endl;
		cout << "<demo option> 2: Using a camera to take pictures" << endl;
	        return -1;
	}

	// default option: the first three images are from originals/0.jpg, originals/1.jpg, and 
	string image1("0");
	string image2("1");
	string image3("2");
	
	if(argc >= 5)  // choose custom images
	{
		int choice = atoi(argv[1]);

		string image1Temp(argv[2]);
		string image2Temp(argv[3]);
		string image3Temp(argv[4]);

		image1 = image1Temp;
		image2 = image2Temp;
		image3 = image3Temp;
	}

	int choice = atoi(argv[1]);

	if (choice == 1) demoOption_1(image1, image2, image3);
	if (choice == 2) demoOption_2(image1, image2, image3);
}

// ***************** Footnotes *************************

// For Demo Option 1 there will be six buttons:
// GPIO 67 Car AIN	GPIO 68 Car AOUT
// GPIO 44 Car BIN	GPIO 26 Car BOUT
// GPIO 46 Car CIN	GPIO 65 Car COUT

//#include <stdio.h>			// run_tess.cpp
//#include <stdlib.h>			// run_tess.cpp
//#include <fstream>			// run_tess.cpp

//#include <cv.h>				// orig_main.cpp && trainSVM.cpp
//#include <highgui.h>		// orig_main.cpp && trainSVM.cpp
//#include <cvaux.h>			// orig_main.cpp && trainSVM.cpp
//#include <ml.h>				// orig_main.cpp

//#include <iostream>			// orig_main.cpp && run_tess.cpp && trainSVM.cpp
//#include <vector>			// orig_main.cpp && trainSVM.cpp	
//#include <string>			// orig_main.cpp && run_tess.cpp

//#include "DetectRegions.h"
//#include "OCR.h"
