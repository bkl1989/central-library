#!/bin/bash
echo "Starting server tests"
export PGPASSWORD="123Password"
psql -U central_library -d central_library -h localhost -f test-database.sql
echo "Ending server tests"
