sudo apt update &> /dev/null
sudo apt install openssh-server &> /dev/null
sudo systemctl enable ssh &> /dev/null
sudo apt install -y postgresql postgresql-contrib &> /dev/null
sudo systemctl enable postgresql &> /dev/null
sudo systemctl start postgresql &> /dev/null
