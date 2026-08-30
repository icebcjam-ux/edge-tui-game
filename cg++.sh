#!/bin/bash

cd /home/red/MultiNode-DevHub/edge-tui-game

g++ -O3 bot_agent.cpp -o bot_agent
g++ -O3 server.cpp -o server
../push_game.sh bot_agent

./server
