#!/bin/bash
echo "Starting server tests"
export PGPASSWORD="123Password"
psql -U central_library -d central_library -h localhost -f test-database.sql
rm helloWorld
g++-13 -std=c++23 helloWorld.cpp -o helloWorld
./helloWorld
rm helloWorld
echo "Ending server tests"
