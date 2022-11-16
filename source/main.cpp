#include <iostream>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include "include/text_processing.h"
#include "../include/localization.h"
#include "../include/stemming/english_stem.h"


using namespace localization;
using namespace std;

int main(int argv, char* argc[])
{
    system("chcp 65001");
    string text = textProcessing::text_load("D:/dbase_en.txt");
    text = textProcessing::clear_text(text);
    vector<string> tokenized_text = textProcessing::strtok_vector(text, ' ');
    stemming::english_stem <> StemEnglish;
    
    cout << tokenized_text[500] << endl;
    wstring temp = strtows(tokenized_text[500], 65001);
    StemEnglish(temp);

    cout << wstostr(temp, 65001) << endl;;
    system("pause");
}