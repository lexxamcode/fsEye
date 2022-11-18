#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <set>
#include <unordered_set>
#include <stemming/stemming.h>
#include "localization.h"
#include "stemming/stemming.h"
#include <stringapiset.h>

namespace textProcessing
{
    std::string stop_symbols = "\t.,/\\=+_<>?'\":;'[]{}!@#$%^&*№1234567890";
    std::regex txt_file("(.*?)(\.txt)|(.*?)(\.dat)");

    std::string text_load(const std::string& path)
    {
        if (std::regex_match(path, txt_file))
        {
            std::string str;
            std::ifstream ifs(path);
            ifs >> std::noskipws;
            std::copy(std::istream_iterator<char>(ifs), std::istream_iterator<char>(), std::back_inserter(str));
            return str;
        }
        else 
            std::cout << "Not a txt or dat file" << std::endl;
        return "";
    }

    bool trash_symbol(const char& symbol)
    {
        return (stop_symbols.find(symbol) != std::string::npos);
    }

    std::string clear_text(std::string &text)
    {
        std::replace(text.begin(), text.end(), '\n', ' ');
        
        text.erase(remove_if(text.begin(), text.end(), trash_symbol), text.end());

        return text;
    }
    //Next method breaks with big data somehow
    std::vector<std::string> tokenize_text(std::string text)
    {
        //std::regex sample("^[A-Za-z-] | ^[А-Яа-я-]"); 
        std::regex sample("[A-Za-z-]+|[А-Яа-я-]+");

        std::sregex_token_iterator iter(text.begin(), text.end(), sample, -1);
        std::sregex_token_iterator end;

        std::vector<std::string> tokens(iter, end); // Breaks with big data

        return tokens;
    }
    //And the next ones don't break with big data
    std::vector<std::string> strtok_vector(std::string str, char delim) {
        std::vector<std::string> tokens;
        std::string temp = "";
        for (int i = 0; i < str.length(); i++) {
            if (str[i] == delim) {
                tokens.push_back(temp);
                temp = "";
            }
            else
                temp += std::tolower(str[i]);
        }
        tokens.push_back(temp);
        return tokens;
    }

    std::set<std::string> strtok_set(std::string str, char delim) {
        std::set<std::string> words;
        std::string temp = "";
        for (int i = 0; i < str.length(); i++) {
            if (str[i] == delim) {
                words.insert(temp);
                temp = "";
            }
            else
                temp += std::tolower(str[i]);
        }
        words.insert(temp);
        return words;
    }

    std::unordered_set<std::string> strtok_uset(std::string str, char delim)
    {
        std::unordered_set<std::string> words;
        std::string temp = "";

        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] == delim)
            {
                words.insert(temp);
                temp = "";
            }
            else
                temp += std::tolower(str[i]);
        }
        words.insert(temp);
        return words;
    }

    std::multiset<std::string> strtok_mset(std::string str, char delim)
    {
        std::multiset<std::string> words;
        std::string temp = "";

        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] == delim)
            {
                words.insert(temp);
                temp = "";
            }
            else
                temp += std::tolower(str[i]);
        }
        words.insert(temp);
        return words;
    }
    
    std::set<std::string> load_text_to_set(std::string filepath)
    {
        std::string text = text_load(filepath);
        clear_text(text);
        return strtok_set(text, ' ');
    }

    std::unordered_set<std::string> load_text_to_uset(std::string filepath)
    {
        std::string text = text_load(filepath);
        clear_text(text);
        return strtok_uset(text, ' ');
    }

    std::multiset<std::string> load_text_to_mset(std::string filepath)
    {
        std::string text = text_load(filepath);
        clear_text(text);
        return strtok_mset(text, ' ');
    }


    std::vector<std::string> load_text_to_vector(std::string filepath)
    {
        std::string text = text_load(filepath);
        clear_text(text);
        return strtok_vector(text, ' ');
    }

    std::string stem_word(std::string& word, stemming::stem<>* stemmer)
    {
        std::wstring temp = localization::strtows(word, 65001);
        stemmer->operator()(temp);
        word = localization::wstostr(temp, 65001);
        return word;
    }
}