cmake_minimum_required(VERSION 3.2)

project(fsEye)

add_executable(fsEye source/main.cpp source/headers/FVectorMaker.h)
set(INCLUDE_DIR include)
target_include_directories(fsEye PRIVATE ${INCLUDE_DIR})