#!/bin/bash
export DATABASE_ROOT="$(./get-config.sh projectRoot)/services/database"
export ARCHIVER_ROOT="$(./get-config.sh projectRoot)/services/archiver"
echo "Starting server tests"
export PGPASSWORD=$(./get-config.sh databasePassword)
SQL_RESULT=$(psql -q -t -A -U central_library -d central_library -h localhost -f $DATABASE_ROOT/test-database.sql 2> /dev/null)
if [[ "$SQL_RESULT" != "hello|world" ]]; then
	exit 1
fi
export PGPASSWORD=""
rm $ARCHIVER_ROOT/helloWorld &> /dev/null
g++-13 -std=c++23 $ARCHIVER_ROOT/helloWorld.cpp -o $ARCHIVER_ROOT/helloWorld
CPP_RESULT=$($ARCHIVER_ROOT/helloWorld)
if [[ "$CPP_RESULT" != "202100" ]]; then
        exit 1
fi
rm $ARCHIVER_ROOT/helloWorld
echo "Server test succeeded"
