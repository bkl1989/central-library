echo "initializing with project root $(./get-config.sh projectRoot)"
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install openssh-server
sudo systemctl enable ssh
sudo apt install -y postgresql postgresql-contrib
sudo systemctl enable postgresql
sudo systemctl start postgresql 
sudo -i -u postgres psql <<EOF
CREATE USER central_library WITH PASSWORD '$(./get-config.sh projectRoot)/config/current/databasePassword' CREATEDB;
CREATE DATABASE central_library OWNER central_library;
EOF
sudo apt install -y g++-13
