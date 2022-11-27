# fsEye
fsEye is a programm for indexing and searching files in a computer's filesystem.
Coursework.

Stemming tools (include/stemming .h files) I took from this repository: https://github.com/Blake-Madden/OleanderStemmingLibrary

# Installation
1. Download set of a libraries Boost from here: https://www.boost.org/users/download/

2. Follow this guide to build Boost: https://www.boost.org/doc/libs/1_62_0/more/getting_started/windows.html#simplified-build-from-source

3. Download DuckX for processing doc/dockx files: https://github.com/amiremohamadi/DuckX
   Also for correct DuckX work you'll need: 
   
   https://github.com/kuba--/zip
   
   https://github.com/zeux/pugixml
   
4. Download sqlite3 library (sqlite-amalgamation-3400000.zip)

https://www.sqlite.org/download.html
4. Download the project:
```bash
git clone https://github.com/lexxamcode/fsEye
```
5. Add path to boost, duckx, zip and pugixml files in CMakeLists.txt:
```cmake
# Boost root and libraries directories are being set here
set(Boost_INCLUDE_DIR YOUR_PATH_TO_BOOST_ROOT)
set(Boost_LIBRARY_DIR YOUR_PATH_TO_BOOST_ROOT/stage/lib)

# DuckX library for doc/docx file processing
set(DINCLUDE_DIR YOUR_PATH_TO_DUCKX_ROOT/include)
set(PUGIXMLDIR YOUR_PATH_TO_PUGIXML_ROOT/src)
set(ZIPDIR YOUR_PATH_TO_ZIP_ROOT/src)
```
# sqlite3 library for working with database
set(SQLDIR YOUR_PATH_TO_SQLITE)
include_directories(${SQLDIR})
link_directories(${SQLDIR})

6. Before building project change one thing in duckx.cpp:
![image](https://user-images.githubusercontent.com/82732974/203646559-395e7f61-18b5-46f7-8c23-d4b7c7d4e8a0.png)
(Hope this issue is solved when someone need this project, but if it's not, just make the change i showed :) )

7. Now build the project:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
# Usage
fsEye is going to be complete separate app, but still, fsEye source files and include files allow user to index text files by content in a directory in different languages:
```cpp
//path to file where indexes will be contained
const string index_file_path = "..\\..\\index.json"; 
//path to the english dictionary. The dictionary is already in project, use this path
const string en_dict_path = "..\\..\\data\\dictionaries\\en_dictionary.txt";
//path to english stopwords. Stopwords are already in project too, use this path.
const string en_stopwords_path = "..\\..\\data\\stopwords\\en_stopwords.txt";

//Then create stemmer that stems words according it's language
stemming::stem<>* stemmer = new stemming::english_stem<>;

//And FVectorMaker that makes the feature vector of given text and language
FVectorMaker en_index_maker(en_dict_path, en_stopwords_path, "en");

//INDEX DIRECTORY
boost::json::object index_json;
indexing::index_entire_directory("D:\\samples", en_index_maker, index_json);
std::ofstream json;
json.open(index_directory);
json << serialize(index_json);
json.close();

//FIND FILE BY CONTENT IN INDEXED DIRECTORY
vector<string> found = indexing::knn_algorithm("I really love algorithms and data structures!", index_directory, en_index_maker, 4);
for (auto& file: found)
    cout << file << endl;
system("pause");
```
