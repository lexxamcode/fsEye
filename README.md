# fsEye
fsEye is a programm for indexing and searching files in a computer's filesystem.
Coursework.

Stemming tools (include/stemming .h files) I took from this repository: https://github.com/Blake-Madden/OleanderStemmingLibrary

# Installation
1. Download set of a libraries Boost from here: https://www.boost.org/users/download/

2. Follow this guide to build Boost: https://www.boost.org/doc/libs/1_62_0/more/getting_started/windows.html#simplified-build-from-source
3. Download the project:
```bash
git clone https://github.com/lexxamcode/fsEye
```
4. Add path to boost in CMakeLists.txt:
```cmake
set(Boost_INCLUDE_DIR YOUR_PATH_TO_BOOST_ROOT)
set(Boost_LIBRARY_DIR YOUR_PATH_TO_BOOST_ROOT/stage/lib)
```
5. Now build the project:
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
