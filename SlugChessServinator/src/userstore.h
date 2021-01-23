#pragma once
#include "../include/nlohmann/json.hpp"
#include <mutex>
#include "../include/plusaes/plusaes.hpp"
#include "../include/picosha2.h"
#include "../include/elo.hpp"
#include "../../SlugChessCore/src/slugchess.h"
#include "messenger.h"
#include "filesystem.h"
class UserStore
{
    

    private:
    static UserStore* _instance;
    UserStore() {  _elo_config = std::make_shared<Elo::Configuration>(32); }

    std::shared_ptr<Elo::Configuration> _elo_config; // K = 32
    std::mutex _mutex; //TODO start using this
    //std::unordered_map<std::string, std::string> _username_to_sha265hash;

    public:
    static UserStore* Get()
    {
        if(!_instance) 
        _instance = new UserStore();
        return _instance;
    }
    // Win Player2 = 1.0, Draw Player2 = 0.5, Loss Player2 = 0.0
    std::pair<double, double> CalcNewElo(double elo_player1, double elo_player2, double result);
    
    static void WriteUserEloToFile(std::shared_ptr<chesscom::UserData> userdata, std::vector<unsigned char>& encryption_key);
    static std::string UsernameToDataFilename(const std::string& username);
    static std::vector<unsigned char> EncryptionKeyFromPassword(const std::string& password);
    static std::string decrypt_string(const std::string& encrypted_string, 
        const std::vector<unsigned char>& encryption_key);
    static std::string encrypt_string(const std::string& data_string, 
        const std::vector<unsigned char>& encryption_key);

    
};
