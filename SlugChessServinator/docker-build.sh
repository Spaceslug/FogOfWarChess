#!/bin/bash
sudo docker build -t grpc-env:latest -f ./Dockerfile.grpc ..
sudo docker build -t slugchess -f ./Dockerfile.servinator ..
