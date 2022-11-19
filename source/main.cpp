#include "headers/FVectorMaker.h"

using namespace std;
using namespace feature_vector;

int main(int argv, char* argc[])
{

    const string dict_path = "../../data/dictionaries/en_dictionary.txt";
    const string stopwords_path = "../../data/stopwords/en_stopwords.txt";
    const string text_path = "../../data/text1.txt";

    stemming::stem<>* stemmer = new stemming::english_stem<>;
    FVectorMaker test_maker(dict_path, stopwords_path, "en");
    FVector a = test_maker.make_feature_vector(text_path);

    cout << "Got text" << endl;
    for (auto &it: a.get_sparse_vector())
        std::cout << it.first << " " << it.second << endl;
    system("pause");
}
