#!/bin/bash

MODE="all"

if [ "$1" != "" ]; then
    MODE="$1"
fi

echo "Stopping in mode: $MODE"

stop_backend() {
    echo "Stopping OJ Server..."
    pkill -f "oj_server"
    
    echo "Stopping Contest Crawler..."
    pkill -f "contest_crawler"
    
    echo "Stopping Compile Servers..."
    pkill -f "compile_server"
}

stop_frontend() {
    echo "Stopping Frontend..."
    # Kill vite server
    pkill -f "vite"
    # Kill npm run dev if it's hanging around
    pkill -f "npm run dev"
}

case $MODE in
    "all")
        stop_backend
        stop_frontend
        ;;
    "backend")
        stop_backend
        ;;
    "frontend")
        stop_frontend
        ;;
    *)
        echo "Usage: $0 [all|backend|frontend]"
        exit 1
        ;;
esac

echo "Shutdown complete."
