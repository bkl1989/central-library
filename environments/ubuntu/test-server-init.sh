#!/bin/bash
export DATABASE_ROOT="$(./get-config.sh projectRoot)/services/database"
export ARCHIVER_ROOT="$(./get-config.sh projectRoot)/services/archiver"
echo "Starting server tests"
export PGPASSWORD=$(./get-config.sh databasePassword)
psql -U central_library -d central_library -h localhost -f $DATABASE_ROOT/test-database.sql
export PGPASSWORD=""
rm helloWorld
g++-13 -std=c++23 $ARCHIVER_ROOT/helloWorld.cpp -o $ARCHIVER_ROOT/helloWorld
$ARCHIVER_ROOT/helloWorld
rm $ARCHIVER_ROOT/helloWorld
echo "Ending server tests"
