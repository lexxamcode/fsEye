#include "headers/FVectorMaker.h"
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/json.hpp>

using namespace std;
using namespace feature_vector;

void index_entire_directory(const string& dir, const FVectorMaker& vector_maker, boost::json::object& main_json_target)
{
    for (const auto& file: boost::filesystem::directory_iterator(dir))
    {
        if (boost::filesystem::is_directory(file))
            index_entire_directory(file.path().string(), vector_maker, main_json_target);
        
        else
        {
            boost::json::object current_file_fvector;
            FVector features = vector_maker.make_feature_vector(file.path().string());
            for (auto& it: features.get_sparse_vector())
            {
                current_file_fvector[to_string(it.first)] = it.second;
            }
            main_json_target[file.path().string()] = current_file_fvector;
            cout << file.path().string() << endl;
        }
    }
}

int main(int argc, char* argv[])
{
    string given_directory = "D:\\samples"; //  by default
    string index_directory = "..\\..\\index.json";
    boost::json::object index_json;

    if (argc == 2)
        given_directory = argv[1];
    else if (argc == 3)
    {
       given_directory = argv[1]; 
       index_directory = argv[2];
       if (index_directory[index_directory.size()] != char("/"))
            index_directory.append("\\index.json");
    }

    const string en_dict_path = "..\\..\\data\\dictionaries\\en_dictionary.txt";
    const string en_stopwords_path = "..\\..\\data\\stopwords\\en_stopwords.txt";
    const string text_path = "..\\..\\data\\text1.txt";

    stemming::stem<>* stemmer = new stemming::english_stem<>;
    FVectorMaker en_index_maker(en_dict_path, en_stopwords_path, "en");

    index_entire_directory(given_directory, en_index_maker, index_json);

    std::ofstream json;
    json.open(index_directory);
    json << serialize(index_json);
    json.close();
}
