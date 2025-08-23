sudo apt update &> /dev/null
sudo apt install openssh-server &> /dev/null
sudo systemctl enable ssh &> /dev/null
sudo apt install -y postgresql postgresql-contrib &> /dev/null
sudo systemctl enable postgresql &> /dev/null
sudo systemctl start postgresql &> /dev/null
sudo -i -u postgres psql <<EOF
CREATE USER central_library WITH PASSWORD '123Password' CREATEDB;
CREATE DATABASE central_library OWNER central_library;
EOF
