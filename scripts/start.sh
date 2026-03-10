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

# Get CPU core count (Linux/macOS compatible)
if command -v nproc >/dev/null 2>&1; then
    CPU_CORES=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    CPU_CORES=$(sysctl -n hw.ncpu)
else
    CPU_CORES=1
fi

# Limit max compile servers to avoid resource exhaustion (e.g. max 8)
if [ "$CPU_CORES" -gt 8 ]; then
    CPU_CORES=8
fi

echo "Detected CPU Cores: $CPU_CORES. Starting $CPU_CORES Compile Servers..."

# Prepare service_machine.conf path
CONF_FILE="$curr_dir/output/oj_server/conf/service_machine.conf"
# Ensure conf directory exists (it should be copied by make output, but just in case)
mkdir -p "$(dirname "$CONF_FILE")"
# Clear existing config
> "$CONF_FILE"

cd output/compile_server

START_PORT=8081
for ((i=0; i<CPU_CORES; i++)); do
    PORT=$((START_PORT + i))
    nohup ./compile_server $PORT > compile_${PORT}.log 2>&1 &
    echo "Compile Server $PORT started (pid $!)"
    
    # Add to config file
    echo "127.0.0.1:$PORT" >> "$CONF_FILE"
done

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
# Create uploads directories if they don't exist
mkdir -p wwwroot/uploads/avatars
# Run in background with nohup
nohup ./oj_server > ../logs/oj_server.log 2>&1 &
echo "OJ Server started (pid $!)"
