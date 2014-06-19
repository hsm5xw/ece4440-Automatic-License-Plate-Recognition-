// name: Harrison Brookeman, Joseph Deguzman, Hong Moon, Zachary Zydron
// date: 2013-12-13
// version: 2.1

#include "DBManager.h"

using namespace std;

DBManager::DBManager()
{
   PAYMENT_UNIT  = 2;
   BaseImagePath = "input_images/";

   struct stat sb;
   
   if( stat(BaseImagePath.c_str() , &sb) == 0 && S_ISDIR(sb.st_mode))
   {
      // it is a directory
      //cout << "BASEIMAGEPATH: The directory exists" << endl; 
   }
   else
   {
      // the directory does not exist
      //cout << "BASEIMAGEPATH: The directory does not exist" << endl;

      // create one with mkdir command

      mode_t process_mask = umask(0);      
      int result_code = mkdir(BaseImagePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
      umask(process_mask);    

      if(result_code < 0)
		cout << "error creating a base image directory" << endl;
      
   }


}

int callback(void *data, int argc, char **argv, char ** ColName){

   for(int i=0; i< argc; i++)
   {
      printf("%s = %s\n",  ColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

string doubleToString(double dbl)
{
   stringstream sstr;
   sstr << dbl;
   return sstr.str();
}

bool ImageExists(const char* filename) {
	
	struct stat fileInfo;
	return stat(filename, &fileInfo) == 0;
}


bool DBManager::insert(string plateNumber, string capturedPath)
{
   sqlite3 *db;

   /* Open database */
   int rc = sqlite3_open("test.db", &db);
   if( rc )
   {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }

   sqlite3_stmt *stmt;   
 
   const char * plate_number = plateNumber.c_str();

   string imagePath = this->getBaseImagePath() + plateNumber + ".jpg";
   //cout << "Image Path is : " << imagePath << endl;
   
   if( sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO CAR( plate_number, admission_time, admission_timestamp) \
                               VALUES(?, DateTime('now', 'localtime'), strftime('%s', 'now', 'localtime') )", 
                               -1 , &stmt, 0) != SQLITE_OK )
   {
      printf("Could not prepare statement \n");
      return false;
   }

   /* Bind a value to the first parameter */
   if ( sqlite3_bind_text( stmt, 1, plate_number, -1 , 0 ) != SQLITE_OK )
   {
      printf("Could not bind the first parameter.\n");
      return false;
   }
 
 
   /* Execute the SQL statement */
   if (sqlite3_step(stmt) != SQLITE_DONE) 
   {
      printf("Insert: Could not step (execute) stmt.\n");
      printf("Insert: The record may already exist \n");
      return false;
   }
   else
   {
      if ( !ImageExists( imagePath.c_str() ) ) 
      {
	// store image to the base image folder if the image has not been stored
	Mat input_image;
	input_image=imread( capturedPath.c_str(),1);
	
	imwrite( imagePath, input_image);
	fprintf(stdout, "Saved image with license plate %s \n", plateNumber.c_str() );
	
      }

      fprintf(stdout, "Record inserted successfully\n");
   }
   sqlite3_finalize(stmt);

   /* Close the database*/
   sqlite3_close(db);

   return true;
}

void DBManager::select()
{
   sqlite3 *db;
   char * ErrorMessage = 0;
   int rc;
   const char *sql;
   const char* data = "Callback function called\n\n";

   /* Open database */
   rc = sqlite3_open("test.db", &db);
   if( rc )
   {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }

   /* Create SQL statement */
   sql = "SELECT * from CAR";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, (void*)data, &ErrorMessage);
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", ErrorMessage);
      sqlite3_free(ErrorMessage);
   }
   else
   {
      //fprintf(stdout, "Operation done successfully\n");
   }

   /* Close the database */
   sqlite3_close(db);
}


vector< vector <string> > DBManager::selectWithWildCard(string plateNumber)
{
   string originalPlateNumber = plateNumber;
   string wildCardPlateNumber = plateNumber;

   //cout << "Printing out all possible wild cards" << endl << endl;

   sqlite3 *db;
   
   /* Open database */
   int rc = sqlite3_open("test.db", &db);
   if( rc )
   {
      	fprintf(stderr, "Cant open database: %s\n", sqlite3_errmsg(db) );
      	exit(0);
   }

   vector< vector<string> > wildCardResults;
   
   for(std::string::size_type i=0; i < originalPlateNumber.size(); i++ )
   {
     	wildCardPlateNumber = wildCardPlateNumber.replace(i, 1, "_"); // replace each character with a wildcard character
 	
	/* open statement */
	sqlite3_stmt *stmt;
	const char * wildCardPlateNumberChar = wildCardPlateNumber.c_str();
	
	const char * q_plate_number;
	const char * q_admission_time;
	double q_timestamp;

	if( sqlite3_prepare_v2(db, "SELECT * FROM CAR WHERE plate_number LIKE ?", -1, &stmt, 0) != SQLITE_OK )
	{
		printf("Could not prepare statement \n");
		exit(0);
	}

	/* Bind a value to the first parameter */
	if( sqlite3_bind_text( stmt, 1, wildCardPlateNumberChar, -1, 0) != SQLITE_OK )
	{
		printf("Could not bind the first parameter. \n");
		exit(0);
	}

	int stmt_numRecords = 0;

	/* Execute the SQL statmenet */
	while( sqlite3_step(stmt) == SQLITE_ROW )
	{
		q_plate_number = (const char *) sqlite3_column_text(stmt, 0);
		q_admission_time = (const char *) sqlite3_column_text(stmt, 1);
		q_timestamp = sqlite3_column_double(stmt, 2);

	
		//cout << "plate_number: "   << q_plate_number << endl;
		//cout << "admission_time: " << q_admission_time << endl;
		//cout << "admission_timestamp" << q_timestamp << endl;
		

		string sq_plate_number( q_plate_number );
		string sq_admission_time( q_admission_time);
		string sq_timestamp = doubleToString( q_timestamp);

		vector<string> eachRow;
		eachRow.push_back( sq_plate_number );
		eachRow.push_back( sq_admission_time );
		eachRow.push_back( sq_timestamp );
		
		wildCardResults.push_back(eachRow);

		stmt_numRecords += 1;		
	}

	wildCardPlateNumber = originalPlateNumber; // restore wildCardPlateNumber to its original

	/* close statements */
	sqlite3_finalize(stmt);
	
   } // end of for-loop

   return wildCardResults;
}


double DBManager::calculate(string plateNumber)
{
   const int PAYMENT_UNIT = 2;    // unit hourly price for parking per hour (in dollars)
   sqlite3 *db;

   /* Open database */
   int rc = sqlite3_open("test.db", &db);
   if( rc )
   {
      	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      	exit(0);
   }
 
   /* Get the admission time of the Car */ 
   sqlite3_stmt *stmt;   
   const char * plate_number = plateNumber.c_str();
   const char * q_admission_time;
   double q_timestamp;
   
   if( sqlite3_prepare_v2(db, "SELECT * FROM CAR WHERE plate_number = ?", -1 , &stmt, 0) != SQLITE_OK )
   {
      	printf("Could not prepare statement \n");
      	exit(0);
   }

   /* Bind a value to the first parameter */
   if ( sqlite3_bind_text( stmt, 1, plate_number, -1 , 0 ) != SQLITE_OK )
   {
      	printf("Could not bind the first parameter.\n");
      	exit(0);
   }

   int stmt_numRecords = 0;

   /* Execute the SQL statement */
   while( sqlite3_step(stmt) == SQLITE_ROW)
   {
      	q_admission_time = (const char *) sqlite3_column_text(stmt,1);
      	q_timestamp = sqlite3_column_double(stmt,2);
      
      	stmt_numRecords += 1;
      	break; // only fetch the first searched item
   }
 
   if(stmt_numRecords != 1)
   {
     	printf("***** No stored record found from the given plate number: %s ***** \n\n", plate_number);
      	//exit(0);
	return -1;
   }
   else
   {
      	//printf( "Queried Timestamp: %f \n" , q_timestamp );
   }

   // Calculate the amount of time (time difference in seconds) that has passed since the admission time of the car 

   sqlite3_stmt * current_time_stmt;   
   double q_current_timestamp;

   if( sqlite3_prepare_v2(db, "SELECT strftime('%s', 'now', 'localtime') AS current_timestamp", -1 , &current_time_stmt, 0) != SQLITE_OK )
   {
      	printf("Could not prepare statement \n");
      	exit(0);
   }

   while( sqlite3_step(current_time_stmt) == SQLITE_ROW)
   {
      	q_current_timestamp = sqlite3_column_double(current_time_stmt,0);
      	break; // only fetch the first searched item
   }

   //printf( "Current Timestamp: %f \n" , q_current_timestamp );

   int num_hours  = 0;
   double parking_rate = 0;

   if(stmt_numRecords == 1)
   {
   	double time_difference = q_current_timestamp - q_timestamp;

        cout << "\nTime Difference: " << time_difference << " seconds" << endl << endl;

   	num_hours = (int) (time_difference / 3600);
   	parking_rate = (num_hours + 1) * PAYMENT_UNIT;

        cout << "***** Customer's Plate Number: \t" << plate_number << endl;
        cout << "***** Admission Time: \t" << q_admission_time << endl;
   }

   cout << "***** Hours parked: \t" << num_hours << " hour(s)" << endl;
   cout << "***** Parking rate: \t$" << parking_rate << endl << endl; 

   /* close statements */
   sqlite3_finalize(stmt);
   sqlite3_finalize(current_time_stmt);

   /* Close database */
   sqlite3_close(db);

   return parking_rate;
}


bool DBManager::deleteRecord(string plateNumber, string capturedPath)
{
   sqlite3 *db;

   /* Open database */
   int rc = sqlite3_open("test.db", &db);
   if( rc )
   {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }

   sqlite3_stmt *stmt;   
 
   const char * plate_number = plateNumber.c_str();
   string imagePath = this->getBaseImagePath() + plateNumber + ".jpg";
   
   if( sqlite3_prepare_v2(db, "DELETE FROM CAR WHERE plate_number = ?", -1 , &stmt, 0) != SQLITE_OK )
   {
      printf("Could not prepare statement \n");
      return false;
   }

   /* Bind a value to the first parameter */
   if ( sqlite3_bind_text( stmt, 1, plate_number, -1 , 0 ) != SQLITE_OK )
   {
      printf("Could not bind the first parameter.\n");
      return false;
   }

   /* Execute the SQL statement */
   if (sqlite3_step(stmt) != SQLITE_DONE) 
   {
      printf("Delete: Could not step (execute) stmt.\n");
      printf("Delete: The record may not exist \n");
      return false;
   }
   else
   {
      if ( ImageExists( imagePath.c_str() ) ) 
      {
	// delete the image from the base image folder if the image exists
	string removeCommand = "rm " + imagePath;
			
	int ret = system(removeCommand.c_str());
	
	if(ret == -1) 
	{
		cout << "Error occurred while running the tesserect" << endl;
   	}

	fprintf(stdout, "deleted the image with license plate %s \n", plateNumber.c_str() );
	
      }

      fprintf(stdout, "Record deleted successfully\n");
   }
   sqlite3_finalize(stmt);

   /* Close the database*/
   sqlite3_close(db);

   return true;
}

double DBManager::getPaymentUnit()
{
	return PAYMENT_UNIT;
}

string DBManager::getBaseImagePath()
{
	return BaseImagePath;
}



