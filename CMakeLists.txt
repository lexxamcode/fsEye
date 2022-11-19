cmake_minimum_required(VERSION 3.2)

project(fsEye)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost root and libraries directories are being set here
set(Boost_INCLUDE_DIR D:/codeprojects/boost_1_80_0)
set(Boost_LIBRARY_DIR D:/codeprojects/boost_1_80_0/stage/lib)
find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIR})

# DuckX library for doc/docx file processing
set(DINCLUDE_DIR D:/codeprojects/DuckX/include)
set(PUGIXMLDIR D:/codeprojects/pugixml/src)
set(ZIPDIR D:/codeprojects/zip/src)
set(INCLUDE_DIR include)
set(DATA_DIR data)

add_executable(fsEyeIndexator source/indexator.cpp 
               source/headers/FVectorMaker.h 
               source/headers/FVector.h)

target_include_directories(fsEyeIndexator PRIVATE ${INCLUDE_DIR})
target_include_directories(fsEyeIndexator PRIVATE ${DATA_DIR})
target_include_directories(fsEyeIndexator PRIVATE ${DINCLUDE_DIR})
target_include_directories(fsEyeIndexator PRIVATE ${PUGIXMLDIR})
target_include_directories(fsEyeIndexator PUBLIC ${ZIPDIR})

target_link_libraries(fsEyeIndexator ${Boost_LIBRARIES})