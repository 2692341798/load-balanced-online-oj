#!/bin/bash

# Ensure we are in the project root directory
cd "$(dirname "$0")/.."

# Start Compile Servers
echo "Starting Compile Servers..."
# Use absolute path or handle relative path carefully
curr_dir=$(pwd)

if [ ! -d "output/compile_server" ]; then
    echo "Error: output/compile_server not found. Please run 'make output' first."
    exit 1
fi

cd output/compile_server
nohup ./compile_server 8081 > compile_8081.log 2>&1 &
echo "Compile Server 8081 started (pid $!)"
nohup ./compile_server 8082 > compile_8082.log 2>&1 &
echo "Compile Server 8082 started (pid $!)"
nohup ./compile_server 8083 > compile_8083.log 2>&1 &
echo "Compile Server 8083 started (pid $!)"

cd "$curr_dir"

# Start Contest Crawler
echo "Starting Contest Crawler..."
if [ ! -d "output/crawler" ]; then
    echo "Error: output/crawler not found. Please run 'make output' first."
    # Don't exit, try to start oj_server
else
    cd output/crawler
    mkdir -p ../logs
    nohup ./contest_crawler > ../logs/crawler.log 2>&1 &
    echo "Contest Crawler started (pid $!)"
    cd "$curr_dir"
fi

# Start OJ Server
echo "Starting OJ Server..."
if [ ! -d "output/oj_server" ]; then
    echo "Error: output/oj_server not found. Please run 'make output' first."
    exit 1
fi

cd output/oj_server
# Run in foreground so we can see output or Ctrl+C to stop (though that won't stop compile servers)
./oj_server
