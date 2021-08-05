#!/bin/bash
rm -rf node_modules/
sudo rm -rf /usr/local/lib/node_modules
sudo rm -rf ~/.npm
sudo dpkg --remove --force-remove-reinstreq node
sudo apt autoremove nodejs
sudo apt-get install nodejs
sudo npm install -g npm@latest
