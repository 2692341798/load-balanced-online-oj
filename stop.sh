#!/bin/bash
# Stop OJ Server and Compile Servers
# Be careful not to kill other processes with similar names if any
# Using pkill with full name matching or path matching if possible

echo "Stopping OJ Server..."
pkill -f "oj_server"

echo "Stopping Contest Crawler..."
pkill -f "contest_crawler"

echo "Stopping Compile Servers..."
pkill -f "compile_server"

echo "All services stopped."
