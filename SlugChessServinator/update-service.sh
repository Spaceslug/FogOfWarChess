#!/bin/bash
sudo systemctl stop slugchess
sudo cp -R output/* /opt/slugchess
sudo chown slugchess:slugchess /opt/slugchess/*
sudo systemctl start slugchess
