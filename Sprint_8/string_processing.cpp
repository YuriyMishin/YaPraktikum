#include "string_processing.h"

using namespace std;

vector<string_view> SplitIntoWords(string_view text) {
    vector<string_view> words;
    while (true){
        const auto space_pos = text.find(' ');
        words.push_back(text.substr(0,space_pos));
        if (space_pos == text.npos){
            break;
        } else {
            text.remove_prefix(space_pos+1);
        }
    }
    return words;
}



