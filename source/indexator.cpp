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
    FVectorMaker en_index_maker(en_dict_path, en_stopwords_path, "en");
    //INDEX DIRECTORY BY CONTENT
    // indexing::index_directory_by_content("D:\\samples", "..\\..\\index.db", en_index_maker);

    //FIND BY CONTENT IN DIRECTORY
    while(1)
    {
        cout << "Enter your request: ";
        string request;
        cin >> request;
        vector<string> found = indexing::knn_algorithm(request, "..\\..\\index.db", en_index_maker, 4);
        cout << found.size() << endl;
        for (auto& file: found)
            cout << file << endl;
    }

    //INDEX DIRECTORY
    // cout << "Indexing Samples" << endl;
    // indexing::index_directory_to_db("D:\\", "..\\..\\index.db");

    // FIND IN DIRECTORY:
    // while(true)
    // {
    //     cout << "Enter your request: ";
    //     string request;
    //     cin >> request;
    //     unordered_set<string> found = indexing::find_in_db(request, "..\\..\\index.db");
    //     for (auto& path: found)
    //         cout << path << endl;
    // }
    system("pause");
}
