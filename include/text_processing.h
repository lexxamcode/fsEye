#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <set>
#include <unordered_set>
#include <sstream>
#include <stringapiset.h>

#include "boost/iostreams/device/mapped_file.hpp"
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range_core.hpp>

#include <duckx.hpp>
#include <zip.h>
#include <pugixml.hpp>
#include "../zip/src/zip.c"
#include "../pugixml/src/pugixml.cpp"
#include "../DuckX/src/duckx.cpp"
#include <duckx.hpp>
#include <duckxiterator.hpp>
#include <stemming/stemming.h>
#include "localization.h"
#include "stemming/stemming.h"

namespace textProcessing
{
    const std::string stop_symbols = " \n\t.,/\\=+_-<>?'\":;'[]{}!@#$%^&*№1234567890";
    const std::regex txt_file("(.*?)(\.txt)|(.*?)(\.rtf)");
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
    std::vector<std::string> string_to_vector(const std::string& str, stemming::stem<>* stemmer)
    {
        std::vector<std::string> words;
        if (str.size() <= 0)
            return words;

        std::string word;
        for (size_t i = 0; i < str.size(); i++)
        {
            if (!trash_symbol(str[i]))
                word.push_back(tolower(str[i]));
            else
            {
                if (!word.empty())
                {
                    stem_word(word, stemmer);
                    words.push_back(word);
                    word.erase();
                }
            }
        }
        if (!word.empty())
                words.push_back(word);

        return words;
    }

    std::vector<std::string> text_to_vector(const std::string& path, stemming::stem<>* stemmer)
    {
        std::vector<std::string> words;

        

        if (std::regex_match(path, txt_file))
        {
            try
            {
                mapped_file fmap(path, mapped_file::readonly);

                auto f = fmap.const_data();
                auto l = f + fmap.size();

                 std::string word;
                while(f && f !=l)
                {
                    if (!trash_symbol(*f))
                        word.push_back(tolower(*f));
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
                if (!word.empty())
                    words.push_back(word);

                return words;
            }
            catch (boost::wrapexcept<std::ios_base::failure>)
            {
                return words;
            }
        }
        else if (std::regex_match(path, doc_file))
        {
            duckx::Document file(path);
            file.open();
            std::string word;
            for (auto p : file.paragraphs())
	            for (auto r : p.runs())
                    for (auto symbol: r.get_text())
                    {
                        if (!trash_symbol(symbol))
                        word.push_back(tolower(symbol));
                        else
                        {
                            if (!word.empty())
                            {
                                stem_word(word, stemmer);
                                words.push_back(word);
                                word.erase();
                            }
                        }
                    }
            if (!word.empty())
                words.push_back(word);

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

            if (!word.empty())
                wordset.insert(word);
            return wordset;
        }
        else
            std::cout << "Not a txt or dat file" << std::endl;
        return wordset;
    }
}