#include <iostream>
#include <windows.h>
#include "../include/tokenize.h"

int main(int argv, char* argc[])
{

    std::string text = "I love big semi-dimentional and huge balls Опа а тут русский полу-текст!";
    std::vector<std::string> tokenized_text = textProcessing::tokenize_text(text);
    for (auto &it : tokenized_text)
        std::cout << it << std::endl;
    system("pause");
    return 0;
}