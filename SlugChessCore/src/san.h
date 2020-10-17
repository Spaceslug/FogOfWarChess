#pragma once
#include <vector>
#include <regex>
#include "gamerules.h"

class San
{

    public:
    static std::vector<std::string> SanMovesFromSan(const std::string& san)
    {
         // default constructor = end-of-sequence:
        std::sregex_token_iterator rend;
        std::vector<std::string> moves;
        std::regex regex(R"(\s*\d+\.\s+)");
        auto suffixes = std::sregex_token_iterator(san.begin(), san.end(), regex, -1);
        suffixes++; // First is garbage ????
        while(suffixes != rend){
            //std::cout << "a" << *suffixes++ << std::endl;
            AddHalfMovesFromFullMove(*suffixes++, moves);
        }

        return moves;
    }

    private:
    static std::sregex_token_iterator FullMovesIter(const std::string& san)
    {
        const std::regex regex(R"(\s*\d+\.\s+)"); 
        return std::sregex_token_iterator(san.begin(), san.end(), regex, -1);
    }

    static void AddHalfMovesFromFullMove(const std::string& fullmove, std::vector<std::string>& moves)
    {
        std::sregex_token_iterator rend;
        const std::regex regex(R"(\S+)"); // non whitespace sequence
        std::sregex_token_iterator matches(fullmove.begin(), fullmove.end(), regex);
        //std::cout << *matches << std::endl;
        moves.push_back(*matches++);
        moves.push_back(*matches++);
    }

};