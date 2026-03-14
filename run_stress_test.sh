#!/bin/bash

# Kill any existing processes
pkill oj_server
pkill compile_server
sleep 1

echo "Starting Compile Servers..."
cd output/compile_server
./compile_server 8081 > ../../compile_8081.log 2>&1 &
./compile_server 8082 > ../../compile_8082.log 2>&1 &
./compile_server 8083 > ../../compile_8083.log 2>&1 &
sleep 2

echo "Starting OJ Server..."
cd ../oj_server
./oj_server > ../../oj_server.log 2>&1 &
sleep 5

echo "Running Stress Test..."
cd ../../tests
./stress_test 20 100 8094

echo "Running Security Test..."
./security_test

echo "Cleaning up..."
pkill oj_server
pkill compile_server
echo "Done."
