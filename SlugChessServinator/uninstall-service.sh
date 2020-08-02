#!/bin/bash
echo "removing SlugChess Server demon"
sudo systemctl stop slugchess
sudo systemctl disable slugchess
sudo rm /etc/systemd/system/slugchess.service
