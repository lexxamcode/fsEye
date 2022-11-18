cmake_minimum_required(VERSION 3.2)

project(fsEye)

add_executable(fsEye source/main.cpp source/headers/FVectorMaker.h source/headers/FVector.h)
set(INCLUDE_DIR include)
set(DATA_DIR data)
target_include_directories(fsEye PRIVATE ${INCLUDE_DIR})
target_include_directories(fsEye PRIVATE ${DATA_DIR})