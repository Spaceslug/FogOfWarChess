#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <iomanip>

class Filesystem
{
    public:
    static std::string RootDir(){
        char buf[4096 + 1];
        if (readlink("/proc/self/exe", buf, sizeof(buf) - 1) == -1)
        throw std::string("readlink() failed");
        std::string str(buf);
        std::string rootDir = str.substr(0, str.rfind('/')+1); //Retuns up to and including last '/'
        return rootDir;
    }

    static void WriteMatchPgn(const std::string userWhite, const std::string& userBlack, const std::string& pgn, time_t& time_t)
    {
        std::ofstream pgnFile;
        std::stringstream ss;ss << RootDir() << "games_database/" << std::put_time(std::gmtime(&time_t), "%Y-%m-%d-%H-%M-%S") << "_" << userWhite << "_" << userBlack << ".pgn";
        pgnFile.open (ss.str(), std::ios::out | std::ios::trunc);
        pgnFile << pgn;
        pgnFile.flush();
    }

};