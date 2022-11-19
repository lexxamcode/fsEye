#include <unordered_set>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include "../../include/text_processing.h"
#include "../../include/stemming/allstems.h"
#include "FVector.h"

using namespace std;
using namespace localization;
using namespace textProcessing;

/* 
    FVectorMaker is a class with contained dictionary
    It provides work with given text, feature vectors,
    Stopwords and more. 
    Awailable languages:
        danish dk 
        dutch nl 
        english en 
        finnish fi
        french fr
        german de
        italian it
        norwegian no
        portuguese pt
        russian ru
        spanish es
*/
class FVectorMaker
{
    private:
        // Set of all words and words that program doesn't need
        vector<string> _dictionary;
        unordered_set<string> _stopwords;
        string _lang;
        stemming::stem<>* _stemmer;

        void set_stemmer()
        {
            if (_lang[0] == 'f')
            {
                switch(_lang[1])
                {
                    case 'i': 
                    {
                        //Finnish stemmer
                        _stemmer = new stemming::finnish_stem<>;
                        break;
                    }
                    case 'r':
                    {
                        //French stemmer
                        _stemmer = new stemming::french_stem<>;
                        break;
                    }
                    default: {_lang = ""; _stemmer = nullptr; break;}
                }
            }
            else if(_lang[0] == 'd')
            {
                switch (_lang[1])
                {
                    case 'k':
                    {
                        //Danish stemmer
                        _stemmer = new stemming::danish_stem<>;
                        break;
                    }
                    case 'e':
                    {
                        //German stemmer
                        _stemmer = new stemming::german_stem<>;
                        break;
                    }
                    default: {_lang = ""; _stemmer = nullptr; break;}
                }
            }
            else if (_lang[0] == 'e')
            {
                switch(_lang[1])
                {
                    case 'n':
                    {
                        //English stemmer
                        _stemmer = new stemming::english_stem<>;
                        break;
                    }
                    case 's':
                    {
                        //Spanish stemmer
                        _stemmer = new stemming::spanish_stem<>;
                        break;
                    }
                    default: {_lang = ""; _stemmer = nullptr; break;}
                }
            }
            else if (_lang[0] == 'n')
            {
                switch (_lang[1])
                {
                    case 'l':
                    {
                        //Dutch stemmer
                        _stemmer = new stemming::dutch_stem<>;
                        break;
                    }
                    case 'o':
                    {
                        //Norwegian stemmer
                        _stemmer = new stemming::norwegian_stem<>;
                        break;
                    }
                    default: {_lang = ""; _stemmer = nullptr; break;}
                }
            }
            else
            {
                if (_lang == "it")
                {
                    //Italian stemmer
                    _stemmer = new stemming::italian_stem<>;
                }
                else if (_lang == "pt")
                {
                    //Portuguese stemmer
                    _stemmer = new stemming::portuguese_stem<>;
                }
                else if (_lang == "ru")
                {
                    //Russian stemmer
                    _stemmer = new stemming::russian_stem<>;
                }
                else
                {
                    _lang = "";
                    _stemmer = nullptr;
                }
            }

        }

        // Steps to prepare given txt files for making feature vector:
        // First step - making vector from txt file of dictionary and set from txt file of stop-words
        // Second step - clearing dictionary from stop-words
        void clear_dict_from_stopwords()
        {
            if (_stopwords.empty() || _dictionary.empty())
                return;
            vector<string> tmp;
            for (auto it = _dictionary.begin(); it != _dictionary.end(); it++)
            {
                if (!_stopwords.count(*it))
                {
                    tmp.push_back(*it);
                }
            }
            _dictionary = tmp;
        }
        // Third step - stemming dictionary  according to the set language: Already stemmed in text_processing.h
        // Fourth step - reduce size of dictionary by erasing identical elements
        void clear_dict_from_identical()
        {
            std::sort(_dictionary.begin(), _dictionary.end());
            _dictionary.erase(std::unique(_dictionary.begin(), _dictionary.end()), _dictionary.end());
        }
        // Now We can Prepare this class for making Feature vector
        void set_ready(const string& init_dict_path, const string& init_stopwords_path, const string& language)
        {
            _lang = language;
            if (_lang.size() != 2) // must be two symbols: "en", "ru", "fr" etc.
                throw - 1;
            set_stemmer();

            _dictionary = text_to_vector(init_dict_path, _stemmer);
            _stopwords = text_to_uset(init_stopwords_path, _stemmer);
            clear_dict_from_stopwords();
            clear_dict_from_identical();
        }
    public:
        /*
            Dictionary as the stopwords must satisfy next conditions:
            - No characters other than the alphabet, - and \n
            - One Word per line
            So any file that is going to be the dictionary or stopwords list will be cleared, tokenized and stemmed
            Awailable languages:
                danish dk 
                dutch nl 
                english en 
                finnish fi
                french fr
                german de
                italian it
                norwegian no
                portuguese pt
                russian ru
                spanish es
                swedish se
        */
        FVectorMaker(const string& init_dict_path, const string& init_stopwords_path, const string& language)
        {
            
            set_ready(init_dict_path, init_stopwords_path, language);
        }
        ~FVectorMaker()
        {
            delete _stemmer;
        }
        string get_lang() const
        {
            return _lang;
        }
        vector<string> get_dictionary() const
        {
            return _dictionary;
        }
        unordered_set<string> get_stopwords() const
        {
            return _stopwords;
        }

        //Feature Vector make function
        feature_vector::FVector make_feature_vector(const string& file_path) const
        {
            multiset<string> text_words;
            vector<string> temp = text_to_vector(file_path, _stemmer);

            for (auto &word: temp)
                text_words.insert(stem_word(word, _stemmer));

            map<size_t, size_t> sparse_vector;
            
            for (int i = 0; i < _dictionary.size(); i++)
            {
                int word_counter = text_words.count(_dictionary[i]);
                if (word_counter > 0)
                    sparse_vector[i] = word_counter;
            }
            feature_vector::FVector result(sparse_vector, _dictionary.size());
            return result;
        }
};