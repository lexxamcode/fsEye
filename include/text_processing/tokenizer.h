#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>

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

    std::vector<std::string> tokenize_text(std::string text)
    {
        //std::regex sample("^[A-Za-z-] | ^[А-Яа-я-]"); 
        std::regex sample("[A-Za-z-]+|[А-Яа-я-]+");

        std::sregex_token_iterator iter(text.begin(), text.end(), sample, -1);
        std::sregex_token_iterator end;

        std::vector<std::string> tokens(iter, end); // Breaks with big data

        return tokens;
    }

    std::vector<std::string> mystrtok(std::string str, char delim) {
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
}