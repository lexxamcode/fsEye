cmake_minimum_required(VERSION 3.2)
project(fsEye)
set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_PRECOMPILED_HEADERS=OFF -std=c++17 -fopenmp")

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# Boost root and libraries directories are being set here
set(Boost_INCLUDE_DIR E:/kursa4_aisd/libs/boost_1_81_0)
set(Boost_LIBRARY_DIR E:/kursa4_aisd/libs/boost_1_81_0/stage/lib)
find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIR})

# DuckX library for doc/docx file processing
set(INCLUDE_DIR include)
set(DATA_DIR data)
set(DINCLUDE_DIR E:/kursa4_aisd/libs/DuckX/include)
set(PUGIXMLDIR E:/kursa4_aisd/libs/pugixml/src)
set(ZIPDIR E:/kursa4_aisd/libs/zip/src)
# set(IMGUI E:/kursa4_aisd/libs/ImGui)

# SQLLite API connection
set(SQLDIR E:/kursa4_aisd/libs/sqlite-amalgamation-3400000)
include_directories(${SQLDIR})
link_directories(${SQLDIR})

add_executable(fsEye source/indexator.cpp 
               source/headers/FVectorMaker.h 
               source/headers/FVector.h
               source/headers/fsWindow.h
               include/imgui/imgui.cpp
               include/imgui/imgui_draw.cpp
               include/imgui/imgui_tables.cpp
               include/imgui/imgui_demo.cpp
               include/imgui/imgui_widgets.cpp
               include/imgui/imgui_impl_dx11.cpp
               include/imgui/imgui_impl_win32.cpp
               include/imgui/imgui_stdlib.cpp
               ${SQLDIR}/sqlite3.c)
            #    include/imgui/backends/imgui_impl_dx9.h
            #    include/imgui/backends/imgui_impl_dx9.cpp
            #    include/imgui/backends/imgui_impl_win32.h
            #    include/imgui/backends/imgui_impl_win32.cpp

target_include_directories(fsEye PRIVATE ${INCLUDE_DIR})
target_include_directories(fsEye PRIVATE ${DATA_DIR})
target_include_directories(fsEye PRIVATE ${DINCLUDE_DIR})
target_include_directories(fsEye PRIVATE ${PUGIXMLDIR})
target_include_directories(fsEye PRIVATE ${ZIPDIR})
target_include_directories(fsEye PRIVATE ${QT_HEADERS_DIR})
# target_include_directories(fsEye PRIVATE ${IMGUI})

target_link_libraries(fsEye ${Boost_LIBRARIES})