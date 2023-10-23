# fsEye
fsEye is a programm for indexing and searching files in a computer's filesystem.
Coursework.

Stemming tools (include/stemming .h files) I took from this repository: https://github.com/Blake-Madden/OleanderStemmingLibrary

# Installation
1. Download set of Boost libraries from here: https://www.boost.org/users/download/

2. Follow this guide to build Boost: https://www.boost.org/doc/libs/1_62_0/more/getting_started/windows.html#simplified-build-from-source

3. Download DuckX for processing doc/dockx files: https://github.com/amiremohamadi/DuckX
   Also for correct DuckX work you'll need: 
   
   https://github.com/kuba--/zip
   
   https://github.com/zeux/pugixml
   
4. Download sqlite3 library (sqlite-amalgamation-3400000.zip)

   https://www.sqlite.org/download.html
   
5. Download the project:
```bash
git clone https://github.com/lexxamcode/fsEye
```
6. Add path to boost, duckx, zip and pugixml files in CMakeLists.txt:
```cmake
# Boost root and libraries directories are being set here
set(Boost_INCLUDE_DIR YOUR_PATH_TO_BOOST_ROOT)
set(Boost_LIBRARY_DIR YOUR_PATH_TO_BOOST_ROOT/stage/lib)

# DuckX library for doc/docx file processing
set(DINCLUDE_DIR YOUR_PATH_TO_DUCKX_ROOT/include)
set(PUGIXMLDIR YOUR_PATH_TO_PUGIXML_ROOT/src)
set(ZIPDIR YOUR_PATH_TO_ZIP_ROOT/src)

# sqlite3 library for working with database
set(SQLDIR YOUR_PATH_TO_SQLITE)
include_directories(${SQLDIR})
link_directories(${SQLDIR})
```

7. Before building project change one thing in duckx.cpp:
![image](https://user-images.githubusercontent.com/82732974/203646559-395e7f61-18b5-46f7-8c23-d4b7c7d4e8a0.png)
(Hope this issue is solved when someone need this project, but if it's not, just make the change i showed :) )

And this one in miniz.h if duckx does not open files with cyrillic symbols in their paths:
![image](https://user-images.githubusercontent.com/82732974/204142792-77feb10a-f604-4441-a786-ad9629e77723.png)


8. Now build the project:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
# Usage - OUTDATED
fsEye is going to be complete separate app, but still, fsEye source files and include files allow user to index text files by content in a directory in different languages:
```cpp
    FVectorMaker en_index_maker(en_dict_path, en_stopwords_path, "en");//Class that processes text and making the feature vector of given language of it
    
    //INDEX DIRECTORY BY CONTENT
    indexing::index_directory_by_content(DIRECTORY, DATABASE_FILENAME, en_index_maker);

    //FIND BY CONTENT IN DIRECTORY
    cout << "Enter your request: ";
    string request;
    cin >> request;
    vector<string> found = indexing::knn_algorithm(request, DATABASE_FILENAME, en_index_maker, k);
    cout << found.size() << endl;
    for (auto& file: found)
        cout << file << endl;

    //INDEX DIRECTORY
    cout << "Indexing Samples" << endl;
    indexing::index_directory_to_db(DIRECTORY, DATABASE_FILENAME);

    // FIND IN DIRECTORY:
    while(true)
    {
        cout << "Enter your request: ";
        string request;
        cin >> request;
        unordered_set<string> found = indexing::find_in_db(request, DATABASE_FILENAME);
        for (auto& path: found)
            cout << path << endl;
    };
```
