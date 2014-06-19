cmake_minimum_required (VERSION 2.6) 

PROJECT(ANPR)
SET(PROJECT_VERSION "0.1")

# Opencv Package
FIND_PACKAGE( OpenCV REQUIRED )
IF (${OpenCV_VERSION} VERSION_LESS 2.3.0)
	MESSAGE(FATAL_ERROR "OpenCV version is not compatible : ${OpenCV_VERSION}")
ENDIF()

# Find Sqlite3 path
FIND_PATH(SQLITE3_INCLUDE_DIR sqlite3.h)
FIND_LIBRARY(SQLITE3_LIBRARY sqlite3)
IF (NOT SQLITE3_INCLUDE_DIR)
	MESSAGE(SEND_ERROR "Failed to find sqlite3.h")
ENDIF()
IF (NOT SQLITE3_LIBRARY)
	MESSAGE(SEND_ERROR "Failed to find the sqlite3 library")
ENDIF()

SET(SRC
 main.cpp
 DetectRegions.cpp
 Plate.cpp
 DBManager.cpp
)

ADD_EXECUTABLE( ${PROJECT_NAME} ${SRC} )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME}  ${OpenCV_LIBS} ${SQLITE3_LIBRARY} )

ADD_EXECUTABLE( trainSVM trainSVM.cpp )
TARGET_LINK_LIBRARIES( trainSVM  ${OpenCV_LIBS} )

ADD_EXECUTABLE( create-table create-table.cpp )
TARGET_LINK_LIBRARIES( create-table ${SQLITE3_LIBRARY} )

SET(RESOURCES
	train
	run.sh
	temp_output
	tmp
	pin_unexport.sh
	input_images
)
foreach(f ${RESOURCES})
file( COPY ${f} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
endforeach(f)
