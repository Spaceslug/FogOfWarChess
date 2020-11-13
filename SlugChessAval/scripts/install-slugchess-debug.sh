#!/bin/bash
echo Warning this will delete all out old SlugChess data
sudo rm -rf /usr/local/slugchess-debug
wget http://spaceslug.no/slugchess-debug/latest/linux-x64/SlugChess_latest.7z
7z x SlugChess_latest.7z 
sudo mv ./SlugChess /usr/local/slugchess-debug
rm SlugChess_latest.7z
sudo chmod -v -R u+rwX,go+rwX /usr/local/slugchess-debug
sudo chmod -v a+x /usr/local/slugchess-debug/SlugChessAval
sudo chmod -v a+x /usr/local/slugchess-debug/SlugChessUpdater
#make symbolic link in /usr/bin/ to SlugChess and SlugChessUpdater
sudo ln -sf /usr/local/slugchess-debug/SlugChessAval /usr/local/bin/slugchess-debug
rm install-slugchess-debug.sh

