// name: Harrison Brookeman, Joseph Deguzman, Hong Moon, Zachary Zydron
// date: 2013-12-13
// version: 2.1

#ifndef DBManager_h
#define DBManager_h

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 
#include <iostream>
#include <string>
#include <sys/stat.h>

#include <vector>
#include <sstream>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


int callback(void *data, int argc, char **argv, char ** ColName);	// callback routine for the select method
string doubleToString(double dbl);					// converts a double variable to a string
bool ImageExists(const char* filename);					// tells whether the file with the specified path exists

class DBManager
{

	private:
		int PAYMENT_UNIT;    			// unit hourly price for parking per hour (in dollars)
		string BaseImagePath; 			// base image path for image storage

	public:
		DBManager(); 						// constructor
		bool insert(string plateNumber, string capturedPath);	// insert the plate number with the admission time into the DB, returns true on success
		void select();						// show every record in the DB
		double calculate(string plateNumber);   		// calculate the parking rate for the car with the given plate number
		bool deleteRecord(string plateNumber, string capturedPath);	// delete the record for the car with the given plate number in the DB
		vector< vector<string> > selectWithWildCard(string plateNumber); /* returns information about all Cars of which license plate numbers differ by
										   only one character. This is done by making SQL select queries using wild card 											   characters */

		double getPaymentUnit();		// returns the base parking rate (which is 2 dollars)
		string getBaseImagePath();		// returns the base image path           									
};

#endif
