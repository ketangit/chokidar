mqtt-broker:
============
  sudo service mosquitto start
  sudo service mosquitto stop

node-red:
=========
  sudo service node-red start
  sudo service node-red stop

chokidard:
==========
  sudo service chokidard start
  sudo service chokidard stop

Update mosquitto conf file
==========================
sudo vi /etc/mosquitto/mosquitto.conf
  pid_file /var/run/mosquitto.pid
  user pi

  port 1883
  protocol mqtt

  listener 9001 127.0.0.1
  protocol websockets

Config init.d script for node-js
================================
wget https://gist.github.com/Belphemur/cf91100f81f2b37b3e94 -O /tmp/node-red
sudo touch /var/log/node-red.log
sudo chown pi /var/log/node-red.log
sudo cp /tmp/node-red /etc/init.d/. 
sudo chmod +x /etc/init.d/node-red
sudo update-rc.d node-red defaults

git commands
============
git commit -am "update comment"
git push
git status
git pull


Prepare Environment
===================
sudo apt-get install devscripts

Build Package
=============
dpkg-buildpackage -uc -us

Install Package
===============
sudo dpkg -i chokidard_0.0.1_armhf.deb

Uninstall Package
=================
sudo dpkg -P chokidard
