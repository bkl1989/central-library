#!/bin/bash
sudo -i -u postgres psql <<EOF
DROP DATABASE IF EXISTS central_library;
DROP USER IF EXISTS central_library;
EOF

