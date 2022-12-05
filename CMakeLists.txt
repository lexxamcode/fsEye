cmake_minimum_required(VERSION 3.2)
project(fsEye)
set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fopenmp")

# Boost root and libraries directories are being set here
set(Boost_INCLUDE_DIR D:/codeprojects/boost_1_80_0)
set(Boost_LIBRARY_DIR D:/codeprojects/boost_1_80_0/stage/lib)
find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIR})

# DuckX library for doc/docx file processing
set(INCLUDE_DIR include)
set(DATA_DIR data)
set(DINCLUDE_DIR D:/codeprojects/DuckX/include)
set(PUGIXMLDIR D:/codeprojects/pugixml/src)
set(ZIPDIR D:/codeprojects/zip/src)

# SQLLite API connection
set(SQLDIR D:/codeprojects/sqlite-amalgamation-3400000)
include_directories(${SQLDIR})
link_directories(${SQLDIR})

add_executable(fsEyeIndexator source/indexator.cpp 
               source/headers/FVectorMaker.h 
               source/headers/FVector.h
               ${SQLDIR}/sqlite3.c)

target_include_directories(fsEyeIndexator PRIVATE ${INCLUDE_DIR})
target_include_directories(fsEyeIndexator PRIVATE ${DATA_DIR})
target_include_directories(fsEyeIndexator PRIVATE ${DINCLUDE_DIR})
target_include_directories(fsEyeIndexator PRIVATE ${PUGIXMLDIR})
target_include_directories(fsEyeIndexator PRIVATE ${ZIPDIR})
#target_include_directories(fsEyeIndexator PRIVATE ${SQLDIR})

target_link_libraries(fsEyeIndexator ${Boost_LIBRARIES})