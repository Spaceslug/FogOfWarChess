#!/bin/bash
echo Warning this will delete all out old SlugChess data
sudo rm -rf /usr/local/slugchess
wget http://spaceslug.no/slugchess/latest/linux-x64/SlugChess_latest.7z
7z x SlugChess_latest.7z 
sudo mv ./SlugChess /usr/local/slugchess
rm SlugChess_latest.7z
sudo chmod -v -R u+rwX,go+rwX /usr/local/slugchess
sudo chmod -v a+x /usr/local/slugchess/SlugChessAval
sudo chmod -v a+x /usr/local/slugchess/SlugChessUpdater
#make symbolic link in /usr/bin/ to SlugChess and SlugChessUpdater
sudo ln -sf /usr/local/slugchess/SlugChessAval /usr/local/bin/slugchess
rm install-slugchess.sh
