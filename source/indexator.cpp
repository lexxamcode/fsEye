#include "indexing.h"

using namespace std;
using namespace feature_vector;

int main(int argc, char* argv[])
{
    cout << "Enter your request: ";
    string request;
    cin >> request;

    const string index_directory = "../../index.json";
    const string en_dict_path = "..\\..\\data\\dictionaries\\en_dictionary.txt";
    const string en_stopwords_path = "..\\..\\data\\stopwords\\en_stopwords.txt";
    const string text_path = "..\\..\\data\\text1.txt";

    stemming::stem<>* stemmer = new stemming::english_stem<>;
    FVectorMaker en_index_maker(en_dict_path, en_stopwords_path, "en");

    // //INDEX DIRECTORY
    // boost::json::object index_json;
    // indexing::index_entire_directory("D:\\samples", en_index_maker, index_json);
    // std::ofstream json;
    // json.open(index_directory);
    // json << serialize(index_json);
    // json.close();

    //FIND IN DIRECTORY
    vector<string> found = indexing::knn_algorithm("japan ships target", index_directory, en_index_maker, 4);
    for (auto& file: found)
        cout << file << endl;
    system("pause");

    // indexing::index_entire_directory(given_directory, en_index_maker, index_json);
}
