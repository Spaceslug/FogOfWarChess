#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unistd.h>
#include <iomanip>
#include <google/protobuf/util/json_util.h>

#include "../include/plusaes/plusaes.hpp"

#include "consts.h"
#include "messenger.h"

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
        std::filesystem::create_directories(RootDir() + "games_database");
        //TODO usernames can be invalid filsystem strings
        std::stringstream ss;ss << RootDir() << "games_database/" << std::put_time(std::gmtime(&time_t), "%Y-%m-%d-%H-%M-%S") << "_" << userWhite << "_" << userBlack << ".pgn";
        pgnFile.open (ss.str(), std::ios::out | std::ios::trunc);
        if(pgnFile.is_open()){
            pgnFile << pgn;
            Messenger::Log("Wrote pgn to '" + ss.str() + "' " + pgn );
        }
        
        pgnFile.close();
    }

    static bool UserFileExists(const std::string& filename)
    {
        std::ifstream userDataFile;
        std::stringstream ss;ss << RootDir() << "user_database/" << filename;
        userDataFile.open (ss.str());
        return userDataFile.is_open();
    }

    // encryprion key must be 32 chars long.
    static void WriteUserFile(chesscom::UserStaticData& userdata, const std::string& filename)
    {
        //std::lock_guard<std::mutex> lock(_user_mutex);
        std::ofstream userFile;
        std::filesystem::create_directories(RootDir() + "user_database");
        std::stringstream ss;ss << RootDir() << "user_database/" << filename;
        userFile.open (ss.str(), std::ios::out | std::ios::trunc);
        if(userFile.is_open()){

            std::string out;
            auto status = google::protobuf::util::MessageToJsonString(userdata, &out);
            if(status.ok()){
                userFile.write(out.data(), out.size());
                Messenger::Log( "Wrote userdata to '" + ss.str() + "' " + out);
            }else{
                Messenger::Log("Something falsed when turning UserStaticData to JSON." + status.error_message().as_string());
                throw std::invalid_argument("encryption of userdata failed");
            }
            
        }else{
            Messenger::Log("Error: Failed to open user data file '" + ss.str() + "'. Failed to save userdata");
            throw std::invalid_argument("encryption of userdata failed");
        }
        
        userFile.close();
    }

    static chesscom::UserStaticData ReadUserFile(const std::string& filename)
    {
        std::ifstream userDataFile;
        std::stringstream ss;ss << RootDir() << "user_database/" << filename;
        userDataFile.open (ss.str(), std::ios::in | std::ios::binary | std::ios::ate ); // ate to move to end of file
        Messenger::Log("opening " + ss.str() + " is opem " + std::to_string(userDataFile.is_open()));
        if(userDataFile.is_open()){
            std::ifstream::pos_type pos = userDataFile.tellg();
            std::vector<unsigned char>  file_data(pos);
            userDataFile.seekg(0, std::ios::beg);
            userDataFile.read((char*)file_data.data(), pos); //Should be fine to cast
            std::string data_string((char*)file_data.data(), pos);
            //Messenger::Log("string: " + data_string);
            // for (auto &&i : decypt_data_string)
            // {
            //     Messenger::Log("'"+ std::string(1,i) +"'");
            // }
            chesscom::UserStaticData de;
            auto status = google::protobuf::util::JsonStringToMessage(data_string, &de);
            if(!status.ok()){
                Messenger::Log(status.error_message());
            }
            userDataFile.close();
            
            //google::protobuf::util::JsonStringToMessage(std::string("{\"username\":\"bleuser\",\"elo\":1568}"), &de);
            return de;
        }
        userDataFile.close();

        Messenger::Log("Error: failed to load userdata");
        throw std::invalid_argument("ekgfekesgo");
        
        return chesscom::UserStaticData();
    }

};