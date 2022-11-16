#include <iostream>
#include <unordered_set>
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
        vector<string> _dictionary;
        unordered_set<string> _stopwords;
        string _lang;

        // Steps to prepare given txt files for making feature vector:
        // First step - making vector from txt file of dictionary and set from txt file of stop-words:
        void loadDictionary(const string& dict_path)
        {
            _dictionary = load_text_to_vector(dict_path);
        }
        void loadStopWords(const string& stopwords_path)
        {
            _stopwords = load_text_to_uset(stopwords_path);
        }
        // Second step - clearing dictionary from stop-words:
        void clear_dict_from_stopwords()
        {
            if (_stopwords.empty() || _dictionary.empty())
                return;
            
            for (auto it = _dictionary.begin(); it != _dictionary.end(); it++)
            {
                if (_stopwords.count(*it))
                    _dictionary.erase(it);
            }
        }
        // Third step - stemming dictionary  according to the set language:
        void stem_dictionary()
        {
            if (_lang[0] == 'f')
            {
                switch(_lang[1])
                {
                    case 'i': 
                }
            }
            if(_lang[0] == 's')
            {

            }
            if(_lang[0] == 'd')
            {

            }
            else
            {
                if (_lang == "en")
                {

                }
                if (_lang == "de")
                {

                }
                if (_lang == "it")
                {

                }
                if (_lang == "no")
                {

                }
                if (_lang == "pt")
                {

                }
                if (_lang == "ru")
                {

                }
                else
                    _lang = "";
            }

        }
        // Fourth step - reduce size of dictionary by erasing identical elements

    public:
         /*
            Dictionary as the stopwords must satisfy next conditions:
            - No characters other than the alphabet, - and \n
            - One Word per line
            So any file that is going to be the dictionary or stopwords list will be cleared, tokenized and stemmed
     */
    FVectorMaker();
    FVectorMaker(const vector<string>& init_dict): _dictionary(init_dict) { }
    FVectorMaker(const string& init_dict_path, const string& init_stopwords_path)
    {
        _dictionary = load_text_to_vector(init_dict_path);     
        _stopwords = load_text_to_uset(init_stopwords_path);
    }
    string get_lang() const
    {
        return _lang;
    }
};