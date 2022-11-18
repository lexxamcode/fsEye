#include "headers/FVectorMaker.h"

using namespace std;

int main(int argv, char* argc[])
{
    const string dict_path = "../../data/dictionaries/en_dictionary.txt";
    const string stopwords_path = "../../data/stopwords/en_stopwords.txt";
    const string text1_path = "../../data/text1.txt";
    const string text2_path = "../../data/text2.txt";
    string text1 = text_load(text1_path);
    string text2 = text_load(text2_path);
    clear_text(text1);
    clear_text(text2);
    cout << text1 << endl;
    system("pause");
    cout << text2 << endl;

    FVectorMaker test_maker(dict_path, stopwords_path, "en");
    vector<string> dict = test_maker.get_dictionary();
    cout << "Dictionary created, EN" << endl;
    system("pause");

    feature_vector::FVector feature_vector1 = test_maker.make_feature_vector(text1_path);
    feature_vector::FVector feature_vector2 = test_maker.make_feature_vector(text2_path);

    cout << "Correlation: " << feature_vector::correlation(feature_vector1, feature_vector2) << endl;
    cout << "Euclidean distance: " << feature_vector::euclidean_distance(feature_vector1, feature_vector1) << endl;
    cout << "Normalized Euclidean distance: " << feature_vector::normalized_euclidean_distance(feature_vector1, feature_vector2) << endl;
    //cout << "Chi Square: " << feature_vector::chi_square(feature_vector1, feature_vector2) << endl;
    cout << "Intersection: " << feature_vector::intersection(feature_vector1, feature_vector2) << endl;
    cout << "Normalized intersection: " << feature_vector::normalized_intersection(feature_vector1, feature_vector2) << endl;
    // for (auto &key: feature_vector1.get_sparse_vector())
    //     cout << key.first << " : " << key.second << endl;
    system("pause");
}
