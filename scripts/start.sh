#!/bin/bash

# Default mode
MODE="all"

# Parse arguments
if [ "$1" != "" ]; then
    MODE="$1"
fi

# Ensure we are in the project root directory
cd "$(dirname "$0")/.."
PROJECT_ROOT=$(pwd)
LOG_DIR="$PROJECT_ROOT/logs"

# Create logs directory
mkdir -p "$LOG_DIR"

echo "Starting in mode: $MODE"

start_compile() {
    echo "Starting Compile Servers..."
    if [ ! -d "output/compile_server" ]; then
        echo "Error: output/compile_server not found. Please run 'make output' first."
        return 1
    fi

    cd output/compile_server
    # Check if already running? Assuming clean state or robust restart needed.
    # We won't kill here, rely on stop.sh
    nohup ./compile_server 8081 > "$LOG_DIR/compile_8081.log" 2>&1 &
    echo "Compile Server 8081 started (pid $!)"
    nohup ./compile_server 8082 > "$LOG_DIR/compile_8082.log" 2>&1 &
    echo "Compile Server 8082 started (pid $!)"
    nohup ./compile_server 8083 > "$LOG_DIR/compile_8083.log" 2>&1 &
    echo "Compile Server 8083 started (pid $!)"
    cd "$PROJECT_ROOT"
}

start_crawler() {
    echo "Starting Contest Crawler..."
    if [ ! -d "output/crawler" ]; then
        echo "Error: output/crawler not found. Please run 'make output' first."
        return 1
    fi
    cd output/crawler
    nohup ./contest_crawler > "$LOG_DIR/crawler.log" 2>&1 &
    echo "Contest Crawler started (pid $!)"
    cd "$PROJECT_ROOT"
}

start_oj() {
    echo "Starting OJ Server..."
    if [ ! -d "output/oj_server" ]; then
        echo "Error: output/oj_server not found. Please run 'make output' first."
        return 1
    fi
    cd output/oj_server
    nohup ./oj_server > "$LOG_DIR/oj_server.log" 2>&1 &
    echo "OJ Server started (pid $!)"
    cd "$PROJECT_ROOT"
}

start_frontend() {
    echo "Starting Frontend..."
    if [ ! -d "frontend" ]; then
        echo "Error: frontend directory not found."
        return 1
    fi
    cd frontend
    # Use nohup to run in background
    nohup npm run dev > "$LOG_DIR/frontend.log" 2>&1 &
    echo "Frontend started (pid $!)"
    cd "$PROJECT_ROOT"
}

case $MODE in
    "all")
        start_compile
        start_crawler
        start_oj
        start_frontend
        ;;
    "backend")
        start_compile
        start_crawler
        start_oj
        ;;
    "frontend")
        start_frontend
        ;;
    "compile")
        start_compile
        ;;
    *)
        echo "Usage: $0 [all|backend|frontend|compile]"
        exit 1
        ;;
esac

echo "Startup complete. Check logs in $LOG_DIR"
