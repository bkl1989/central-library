#!/bin/bash
sudo apt update
sudo apt install openssh-server
sudo systemctl enable ssh
echo "SSH Status:"
sudo systemctl status
