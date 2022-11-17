#include "headers/FVectorMaker.h"

using namespace std;

int main(int argv, char* argc[])
{
    const string dict_path = "../../data/dictionaries/en_dictionary.txt";
    const string stopwords_path = "../../data/stopwords/en_stopwords.txt";
    const string text_path = "../../test.txt";
    string text = text_load(text_path);
    cout << text << endl;

    FVectorMaker test_maker(dict_path, stopwords_path, "en");
    vector<string> dict = test_maker.get_dictionary();
    cout << "Dictionary created, EN" << endl;

    map<int, int> feature_vector = test_maker.make_feature_vector(text_path);

    for (auto &key: feature_vector)
        cout << key.first << " : " << key.second << endl;
    system("pause");
}
