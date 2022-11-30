cmake_minimum_required(VERSION 3.2)

project(fsEye)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost root and libraries directories are being set here
set(Boost_INCLUDE_DIR C:/files/frameworks_and_libs/boost_1_80_0/boost_1_80_0)
set(Boost_LIBRARY_DIR C:/files/frameworks_and_libs/boost_1_80_0/boost_1_80_0/stage/lib)
find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIR})

# DuckX library for doc/docx file processing
set(INCLUDE_DIR include)
set(DATA_DIR data)
set(DINCLUDE_DIR C:/files/frameworks_and_libs/DuckX/include)
set(PUGIXMLDIR C:/files/frameworks_and_libs/pugixml/src)
set(ZIPDIR C:/files/frameworks_and_libs/zip/src)

# SQLLite API connection
set(SQLDIR C:/files/frameworks_and_libs/sqlite-amalgamation-3400000)
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