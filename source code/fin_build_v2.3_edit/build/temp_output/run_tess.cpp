#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main( int argc, char *argv[] ) {
	
	if(argc != 2) {
		cout << "Please enter the image name in the command line argument" << endl;
      		return -1;
	}
    
   	string imageName = argv[1];

   	string s = "tesseract " + imageName + " out";
   	int ret = system( s.c_str() );
  
   	if(ret == -1) {
		cout << "Error occurred while running the tesserect" << endl;
   	}

   	string license_number;
   	ifstream infile;
   	infile.open("out.txt");

	getline(infile, license_number); 
	
	// Logical License Plate String Operation Additions
		
		string space = " ";		
		int space_loc = license_number.find(space);
		if (space_loc > -1) license_number.erase(space_loc, 1);
		
		string dash = "-";
		int dash_loc = license_number.find(dash);
		if (dash_loc > 3) license_number.erase(3, dash_loc-3);

		int size = license_number.size();
		if (size > 8 && license_number.at(size-1)=='1') license_number.erase(size-1, 1);
		
	// End of Operation Additions


	cout<< license_number << endl; 
    
   	infile.close();
   
   return 0;
}
