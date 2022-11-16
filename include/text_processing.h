#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <set>
#include <unordered_set>
#include <stemming/stemming.h>

namespace textProcessing
{
    std::string stop_symbols = "\t.,/\\=+_<>?'\":;'[]{}!@#$%^&*№1234567890";
    std::regex txt_file("(.*?)(\.txt)");

    std::string text_load(std::string path)
    {
        if (std::regex_match(path, txt_file))
        {
            std::string str;
            std::ifstream ifs(path);
            ifs >> std::noskipws;
            std::copy(std::istream_iterator<char>(ifs), std::istream_iterator<char>(), std::back_inserter(str));
            return str;
        }
        else std::cout << "Not a txt file" << std::endl;
    }

    std::string clear_text(std::string &text)
    {
        std::replace(text.begin(), text.end(), '\n', ' ');
        for (auto it = text.begin(); it != text.end(); it++)
        {
            if (stop_symbols.find(*it) != std::string::npos)
                text.erase(it);
        }

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
                temp += str[i];
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
                temp += str[i];
        }
        words.insert(temp);
        return words;
    }

    std::unordered_set<std::string> strtok_uset(std::string str, char delim)
    {
        std::unordered_set<std::string> words;
        std::string temp = "";

        for (int i; i < str.length(); i++)
        {
            if (str[i] == delim)
            {
                words.insert(temp);
                temp = "";
            }
            else
                temp += str[i];
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


    std::vector<std::string> load_text_to_vector(std::string filepath)
    {
        std::string text = text_load(filepath);
        clear_text(text);
        return strtok_vector(text, ' ');
    }

    std::string stem_word(std::string& word, stemming::stem<>* stemmer)
    {
        wstring temp = strtows(word, 65001);
        stemmer->operator()(temp);
        word = wstostr(temp, 65001);
        return word;
    }
}