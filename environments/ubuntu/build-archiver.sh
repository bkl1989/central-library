#!/bin/bash
export ARCHIVER_ROOT="$(./get-config.sh projectRoot)/services/archiver"
rm $ARCHIVER_ROOT/archiver &> /dev/null
g++-13 -std=c++23 -c $ARCHIVER_ROOT/parser.cpp
g++-13 -std=c++23 -c $ARCHIVER_ROOT/archiver.cpp
g++-13 -std=c++23 -c $ARCHIVER_ROOT/jsonParser.cpp
g++-13 -std=c++23 archiver.o parser.o jsonParser.o -o "$ARCHIVER_ROOT/archiver"
rm *.o