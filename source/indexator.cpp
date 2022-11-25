#include "indexing.h"

using namespace std;
using namespace feature_vector;

void load_dictionary()
{
    const string en_dict_path = "..\\..\\data\\dictionaries\\en_dictionary.txt";
    const string en_stopwords_path = "..\\..\\data\\stopwords\\en_stopwords.txt";
    stemming::stem<>* stemmer = new stemming::english_stem<>;
    string dict = test::text_load(en_dict_path);
    test::clear_text(dict);
    vector<string> words = test::strtok_vector(dict, ' ');

    delete stemmer;
}

int main(int argc, char* argv[])
{
    const string index_directory = "../../index.json";
    const string en_dict_path = "..\\..\\data\\dictionaries\\en_dictionary.txt";
    const string en_stopwords_path = "..\\..\\data\\stopwords\\en_stopwords.txt";
    const string text_path = "..\\..\\data\\text1.txt";

    //time_t start = time(NULL);
    //FVectorMaker en_index_maker(en_dict_path, en_stopwords_path, "en");

    // //INDEX DIRECTORY BY CONTENT
    // boost::json::object index_json;
    // indexing::index_by_content_entire_directory("D:\\samples", en_index_maker, index_json);
    // std::ofstream json;
    // json.open(index_directory);
    // json << serialize(index_json);
    // json.close();

    //FIND BY CONTENT IN DIRECTORY
    // cout << "Enter your request: ";
    // string request;
    // cin >> request;
    // vector<string> found = indexing::knn_algorithm(request, index_directory, en_index_maker, 4);
    // cout << found.size() << endl;
    // for (auto& file: found)
    //     cout << file << endl;

    //INDEX DIRECTORY
    // cout << "Indexing Samples" << endl;
    // indexing::index_directory_to_sql("D:\\files", "..\\..\\index.db");

    //FIND IN DIRECORY
    // cout << "Enter your request: ";
    // string request;
    // cin >> request;
    // map<string, string> files = indexing::read_filenames_from_json("../../index.json");
    // string found = "";
    // try 
    // {
    //     found = files["TEST.txt"];
    // }
    // catch(const out_of_range&){ }
    // cout << endl << found << endl;

    // FIND IN SYSTEM:
    while(true)
    {
        cout << "Enter your request: ";
        string request;
        cin >> request;
        unordered_set<string> found = indexing::find_in_db(request, "..\\..\\index.db");
        for (auto& path: found)
            cout << path << endl;
    }

    // indexing::index_entire_directory(given_directory, en_index_maker, index_json);
}
