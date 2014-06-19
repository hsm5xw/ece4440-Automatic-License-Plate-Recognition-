// name: Harrison Brookeman, Joseph Deguzman, Hong Moon, Zachary Zydron
// date: 2013-12-13
// version: 2.1

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 
#include <sys/stat.h>

static int callback(void *NotUsed, int argc, char **argv, char ** ColName)
{
   int i;
   for(i=0; i< argc; i++){
      printf("%s = %s\n", ColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(int argc, char* argv[])
{
   sqlite3 *db;
   char * ErrorMessage = 0;
   int  rc;
   const char *sql;

   /* Open database */
   rc = sqlite3_open("test.db", &db);
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      //fprintf(stdout, "Opened database successfully\n");
   }

   /* Create SQL statement */

   sql = "CREATE TABLE IF NOT EXISTS CAR("  \
         "plate_number        VARCHAR(8) PRIMARY KEY     NOT NULL," \
	 "admission_time      TEXT	 NOT NULL," \
         "admission_timestamp INTEGER    NOT NULL);";

   /* Execute SQL statement */

   rc = sqlite3_exec(db, sql, callback, 0, &ErrorMessage);
   
   if( rc != SQLITE_OK )
   {
   	fprintf(stderr, "SQL error: %s\n", ErrorMessage);
      	sqlite3_free(ErrorMessage);
   }
   else
   {
      	//fprintf(stdout, "Table created successfully\n");
   }
    
   /* Close the database */
   sqlite3_close(db);
   return 0;
}
