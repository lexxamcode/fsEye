#include "headers/FVectorMaker.h"

using namespace std;

int main(int argv, char* argc[])
{
    FVectorMaker test_maker("../../data/dictionaries/en_dictionary.txt", "../../data/stopwords/en_stopwords.txt", "en");
    vector<string> dict = test_maker.get_dictionary();
    cout << "Dictionary: " << endl;
    cout << dict.size();
    for (auto &it: dict)
        cout << it << endl;
    system("pause");

    unordered_set<string> stopwords = test_maker.get_stopwords();

    cout << stopwords.size();
    for (auto &it: stopwords)
        cout << it << endl;
    system("pause");

    string lang = test_maker.get_lang();
    cout << lang << endl;
    system("pause");
}
