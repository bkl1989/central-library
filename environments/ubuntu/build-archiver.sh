#!/bin/bash
export ARCHIVER_ROOT="$(./get-config.sh projectRoot)/services/archiver"
rm $ARCHIVER_ROOT/archiver &> /dev/null
g++-13 -std=c++23 $ARCHIVER_ROOT/archiver.cpp -o $ARCHIVER_ROOT/archiver

