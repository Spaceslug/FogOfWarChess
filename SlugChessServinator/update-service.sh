#!/bin/bash
sudo systemctl stop slugchess
sudo cp output/server /opt/slugchess/server
sudo chown slugchess:slugchess /opt/slugchess/server
sudo systemctl start slugchess
