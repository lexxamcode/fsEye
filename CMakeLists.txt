cmake_minimum_required(VERSION 3.2)

project(fsEye)

# Boost root and libraries directories are being set here
set(Boost_INCLUDE_DIR D:/codeprojects/boost_1_80_0)
set(Boost_LIBRARY_DIR D:/codeprojects/boost_1_80_0/stage/lib)
find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIR})

set(INCLUDE_DIR include)
set(DATA_DIR data)

add_executable(fsEye source/main.cpp 
               source/headers/FVectorMaker.h 
               source/headers/FVector.h)
               
target_include_directories(fsEye PRIVATE ${INCLUDE_DIR})
target_include_directories(fsEye PRIVATE ${DATA_DIR})

target_link_libraries(fsEye ${Boost_LIBRARIES})