#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <set>
#include <unordered_set>
#include <sstream>

#include "boost/iostreams/device/mapped_file.hpp"

#include <stemming/stemming.h>
#include "localization.h"
#include "stemming/stemming.h"
#include <stringapiset.h>

namespace textProcessing
{
    const std::string stop_symbols = "\n\t.,/\\=+_-<>?'\":;'[]{}!@#$%^&*№1234567890";
    const std::regex txt_file("(.*?)(\.txt)|(.*?)(\.dat)");
    const std::regex doc_file("(.*?)(\.doc)|(.*?)(\.docx)");

    using namespace boost::iostreams;

    bool trash_symbol(const char& symbol){ return (stop_symbols.find(symbol) != std::string::npos); }

    std::string stem_word(std::string& word, stemming::stem<>* stemmer)
    {
        std::wstring temp = localization::strtows(word, 65001);
        stemmer->operator()(temp);
        word = localization::wstostr(temp, 65001);
        return word;
    }

    std::vector<std::string> text_to_vector(const std::string& path, stemming::stem<>* stemmer)
    {
        std::vector<std::string> words;

        if (std::regex_match(path, txt_file))
        {
            mapped_file fmap(path, mapped_file::readonly);
            auto f = fmap.const_data();
            auto l = f + fmap.size();

            std::string word;
            while(f && f !=l)
            {
                if (!trash_symbol(*f))
                    word.push_back(*f);
                else
                {
                    if (!word.empty())
                    {
                        stem_word(word, stemmer);
                        words.push_back(word);
                        word.erase();
                    }
                }
                f++;
            }
            return words;
        }
        else
            std::cout << "Not a txt or dat file" << std::endl;
        return words;
    }
    std::unordered_set<std::string> text_to_uset(const std::string& path, stemming::stem<>* stemmer)
    {
        std::unordered_set<std::string> wordset;

        if (std::regex_match(path, txt_file))
        {
            mapped_file fmap(path, mapped_file::readonly);
            auto f = fmap.const_data();
            auto l = f + fmap.size();

            std::string word;
            while(f && f !=l)
            {
                if (!trash_symbol(*f))
                    word.push_back(*f);
                else
                {
                    if (!word.empty())
                    {
                        stem_word(word, stemmer);
                        wordset.insert(word);
                        word.erase();
                    }
                }
                f++;
            }
            return wordset;
        }
        else
            std::cout << "Not a txt or dat file" << std::endl;
        return wordset;
    }
}