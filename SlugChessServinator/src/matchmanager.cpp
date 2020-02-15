#include "matchmanager.h"
#include <random>
#include <exception>

std::string MatchManager::CreateMatch(std::string& player1Token, std::string& player2Token)
{
    std::string matchToken = "match"+ std::to_string(_tokenCounter++);
    std::string white;
    std::string black; 
    std::tie(white, black) = RandomSort(player1Token, player2Token);
    std::shared_ptr<::Match> match = std::make_shared<::Match>(matchToken, white, black,	 
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1", defaultVisionRules);

    match->clock->blackSecLeft = defaultTimeRules.playertime().minutes() * 60 + defaultTimeRules.playertime().seconds();
    match->clock->whiteSecLeft = match->clock->blackSecLeft;
    _matches[matchToken] = match;
    std::cout << "  checing match " << " black sec left " << std::to_string(match->clock->blackSecLeft) << " white sec left " << std::to_string(match->clock->whiteSecLeft) << std::endl << std::flush;
    auto matPtr = _matches[matchToken];
    std::cout << "  white player " <<  matPtr->whitePlayer << std::endl << std::flush;
    return matchToken;
}
std::string MatchManager::CreateMatch(chesscom::HostedGame& hostedGame)
{
    std::string matchToken = "match"+ std::to_string(_tokenCounter++);
    std::string white;
    std::string black;
    if(hostedGame.gamerules().sidetype() == chesscom::SideType::HostIsWhite)
    {
        white = hostedGame.host().usertoken();black = hostedGame.joiner().usertoken();
    }
    else if(hostedGame.gamerules().sidetype() == chesscom::SideType::HostIsBlack)
    {
        black = hostedGame.host().usertoken();white = hostedGame.joiner().usertoken();
    }
    else
    {
        std::tie(white, black) = RandomSort(hostedGame.host().usertoken(), hostedGame.joiner().usertoken());
    }
    std::string fenString;
    if(hostedGame.gamerules().chesstype() == chesscom::ChessType::Classic)
    {
        fenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1";
    }
    else if(hostedGame.gamerules().chesstype() == chesscom::ChessType::FisherRandom)
    {
        //throw std::exception("not implemten FisherRandom chess");
    }
    else if(hostedGame.gamerules().chesstype() == chesscom::ChessType::SlugRandom)
    {
        //throw std::exception("not implemten SlugRandom chess");
    }
    
    VisionRules visionRules = FromChesscomVisionRules(hostedGame.gamerules().visionrules());
    std::shared_ptr<::Match> match = std::make_shared<::Match>(matchToken, white, black, 
       fenString, visionRules);
    match->clock->blackSecLeft = hostedGame.gamerules().timerules().playertime().minutes() * 60 + hostedGame.gamerules().timerules().playertime().seconds();
    match->clock->whiteSecLeft = match->clock->blackSecLeft;
    {
        std::unique_lock<std::mutex> lk(_matchesMutex);
        _matches[matchToken] = match;
    }

    return matchToken;
}

std::shared_ptr<Match> MatchManager::GetMatch(const std::string& matchId)
{
    std::unique_lock<std::mutex> lk(_matchesMutex);
    if(_matches.count(matchId) > 0)
    {
        return _matches[matchId];
    }else{
        return std::shared_ptr<Match>();
    }
}

void MatchManager::EraseMatch(const std::string& matchId)
{
    std::unique_lock<std::mutex> lk(_matchesMutex);
    _matches.erase(matchId);
}

VisionRules MatchManager::FromChesscomVisionRules(const chesscom::VisionRules& chesscomVision)
{
    VisionRules visionRules;
    visionRules.enabled = chesscomVision.enabled();
    visionRules.globalRules.ViewMoveFields = chesscomVision.viewmovefields();
    visionRules.globalRules.ViewRange = chesscomVision.viewrange();
    visionRules.globalRules.ViewCaptureField = chesscomVision.viewcapturefield();
    //std::cout << " Vision rules" << std::endl << std::flush;
    for (auto&& piceRulesPar : chesscomVision.piceoverwriter()) {
        ChessPice pice = (ChessPice)piceRulesPar.first;
        visionRules.overWriteRules[pice].ViewRange = piceRulesPar.second.viewrange();
        visionRules.overWriteRules[pice].ViewMoveFields = piceRulesPar.second.viewmovefields();
        visionRules.overWriteRules[pice].ViewCaptureField = piceRulesPar.second.viewcapturefield();
    }

    return visionRules;
}

std::pair<std::string,std::string> MatchManager::RandomSort(const std::string& first,const std::string& second)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 1);
    if(dist(mt) == 1)
    {
        return {first, second};
    }
    else
    {
        return {second, first};
    }
}