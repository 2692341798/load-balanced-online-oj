#!/bin/bash

# Kill existing processes
echo "Stopping existing services..."
pkill -9 oj_server
pkill -9 compile_server

# Create logs directory if not exists
mkdir -p output/logs

# Start Compile Servers
echo "Starting Compile Server on 8081..."
cd output/compile_server
nohup ./compile_server 8081 > ../logs/compile_server_8081.log 2>&1 &

echo "Starting Compile Server on 8082..."
nohup ./compile_server 8082 > ../logs/compile_server_8082.log 2>&1 &

echo "Starting Compile Server on 8083..."
nohup ./compile_server 8083 > ../logs/compile_server_8083.log 2>&1 &

# Return to root
cd ../..

# Start OJ Server
echo "Starting OJ Server on 8094..."
cd output/oj_server
nohup ./oj_server > ../logs/oj_server.log 2>&1 &

echo "Services started."
