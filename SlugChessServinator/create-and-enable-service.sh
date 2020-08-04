#!/bin/bash
sudo systemctl disable slugchess
sudo systemctl stop slugchess
if id "slugchess" >/dev/null 2>&1; then
        echo "slugchess user allready exists"
else
        echo "creating slugchess user"
        sudo useradd -r -s /sbin/nologin slugchess
fi
echo "Moving SlugChess Server files to /opt/slugchess"
sudo mkdir -p /opt/slugchess
sudo rm -R /opt/slugchess/*
sudo cp -R output /opt/slugchess
sudo chown -R slugchess:slugchess /opt/slugchess
echo "Installing and starting SlugChess Server demon"
sudo cp slugchess.service /etc/systemd/system/slugchess.service
sudo systemctl enable slugchess
sudo systemctl start slugchess
sleep 1
sudo systemctl status slugchess
