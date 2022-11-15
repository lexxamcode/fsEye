#include <iostream>
#include <set>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include "../../include/text_processing/tokenizer.h"
#include "../../include/localization.h"
#include "../../include/stemming/english_stem.h"

using namespace std;
using namespace localization;
using namespace textProcessing;

// FVectorMaker is a class with contained dictionary
// It provides work with given text, feature vectors,
// Stopwords and more.
class FVectorMaker
{
    private:
        // Set of all words and words that program doesn't need
        set<string> dictionary;
        set<string> stopwords;
    public:
     /*
            Dictionary as the stopwords must satisfy next conditions:
            - No characters other than the alphabet, - and \n
            - One Word per line
            So any file that is going to be the dictionary or stopwords list will be cleared, tokenized and stemmed
     */
    FVectorMaker();
    FVectorMaker(const set<string>& init_dict);
    FVectorMaker(const set<string>& init_dict, const set<string>& init_stopwords);

    void setDictionary(const string& dict_path);
    void setDictionary(const set<string>& dict);
};