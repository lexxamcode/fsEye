#include <iostream>
#include <vector>
#include <string>
#include <regex>

namespace textProcessing
{
    std::vector<std::string> tokenize_text(std::string text)
    {
        //std::regex sample("^[A-Za-z-] | ^[А-Яа-я-]"); 
        std::regex sample("[A-Za-z-]+ | [А-Яа-я-]+");

        std::sregex_token_iterator iter(text.begin(), text.end(), sample);
        std::sregex_token_iterator end;

        std::vector<std::string> tokens(iter, end);

        return tokens;
    }
}