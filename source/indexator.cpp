#include "indexing.h"

using namespace std;
using namespace feature_vector;

int main(int argc, char* argv[])
{
    //string given_directory = "D:\\samples"; //  by default
    string index_directory = "../../index.json";
    boost::json::object index_json;

    if (argc == 2)
        index_directory = argv[1];
    else if (argc == 3)
    {
       cout << "Use: fsEyeIndexator <file>.json";
       return 0;
    }

    const string en_dict_path = "..\\..\\data\\dictionaries\\en_dictionary.txt";
    const string en_stopwords_path = "..\\..\\data\\stopwords\\en_stopwords.txt";
    const string text_path = "..\\..\\data\\text1.txt";

    stemming::stem<>* stemmer = new stemming::english_stem<>;
    FVectorMaker en_index_maker(en_dict_path, en_stopwords_path, "en");

    map<string, FVector> files_and_fvectors = indexing::read_from_json(index_directory, en_index_maker.get_dict_size());
    for (auto& it: files_and_fvectors)
    {
        cout << it.first << endl;
        for (auto& index: it.second.get_sparse_vector())
        {
            cout << "\t" << index.first << ": " << index.second << endl;
        }
    }
    system("pause");

    // indexing::index_entire_directory(given_directory, en_index_maker, index_json);

    // std::ofstream json;
    // json.open(index_directory);
    // json << serialize(index_json);
    // json.close();
}
