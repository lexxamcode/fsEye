#include "headers/FVectorMaker.h"

using namespace std;

int main(int argv, char* argc[])
{
    FVectorMaker test_maker("../data/dictionaries/en_dictionary.txt", "../data/stopwords/en_stopwords.txt", "en");
    vector<string> dict = test_maker.get_dictionary();
    unordered_set<string> stopwords = test_maker.get_stopwords();
    string lang = test_maker.get_lang();
    system("pause");
}