#include "userstore.h"
UserStore* UserStore::_instance = nullptr;

std::string UserStore::UsernameToDataFilename(const std::string& username)
{
    std::string data_filename;
    picosha2::hash256_hex_string(username, data_filename);
    return data_filename;
}

std::vector<unsigned char> UserStore::EncryptionKeyFromPassword(const std::string& password)
{
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(password.begin(), password.end(), hash.begin(), hash.end());
    return hash;
}

// Win Player2 = 1.0, Draw Player2 = 0.5, Loss Player2 = 0.0
std::pair<double, double> UserStore::CalcNewElo(double elo_player1, double elo_player2, double result)
{
    Elo::Player player1(elo_player1, *_elo_config);
    Elo::Player player2(elo_player2, *_elo_config);
    Elo::Match match(player1, player2, result, true);
    //match.apply();
    return std::pair<double, double>(player1.rating, player2.rating);

}

void UserStore::WriteUserEloToFile(std::shared_ptr<chesscom::UserData> userdata, std::vector<unsigned char>& encryption_key)
{
    auto dfName = UsernameToDataFilename(userdata->username());
    auto userStatic = Filesystem::ReadUserFile(dfName);
    userStatic.set_elo(userdata->elo());
    Filesystem::WriteUserFile(userStatic, dfName);
}

std::string UserStore::decrypt_string(const std::string& encrypted_string, const std::vector<unsigned char>& encryption_key){
    if(encryption_key.size() != 32){
        Messenger::Log("ERROR: encryption_key must be 32 char long");
        throw std::invalid_argument("encryption_key must be 32 char long");
    }
    const unsigned char iv[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    unsigned long padded_size = 0;
    std::vector<unsigned char> decrypted(encrypted_string.size());
    Messenger::Log("Decrypting: " + encrypted_string);
    plusaes::decrypt_cbc((unsigned char*)encrypted_string.data(), encrypted_string.size(), &encryption_key[0], encryption_key.size(), &iv, &decrypted[0], decrypted.size(), &padded_size);
    return std::string((char*)decrypted.data());
}

std::string UserStore::encrypt_string(const std::string& data_string, const std::vector<unsigned char>& encryption_key){
    if(encryption_key.size() != 32){
        Messenger::Log("ERROR: encryption_key must be 32 char long");
        throw std::invalid_argument("encryption_key must be 32 char long");
    }
    const unsigned char iv[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    // encrypt
    const unsigned long encrypted_size = plusaes::get_padded_encrypted_size(data_string.size());
    std::vector<unsigned char> encrypted(encrypted_size);
    plusaes::encrypt_cbc((unsigned char*)data_string.data(), data_string.size(), &encryption_key[0], encryption_key.size(), &iv, &encrypted[0], encrypted.size(), true);
    return std::string(encrypted.begin(), encrypted.end());
    
}