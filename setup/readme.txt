mqtt-broker:
============
  sudo /etc/init.d/mosquitto start
  sudo /etc/init.d/mosquitto stop

node-red:
=========
  sudo /etc/init.d/node-red start
  sudo /etc/init.d/node-red stop

chokidar:
=========
  sudo /etc/init.d/choki-dar start
  sudo /etc/init.d/choki-dar stop


Update mosquitto conf file
==========================
sudo vi /etc/mosquitto/mosquitto.conf
  pid_file /var/run/mosquitto.pid
  user pi

  port 1883
  protocol mqtt

  listener 9001 127.0.0.1
  protocol websockets

  listener 9009 127.0.0.1
  protocol websockets
  
Config init.d script for choki-dar
==================================
sudo touch /var/log/choki-dar.log
sudo chown ${USER} /var/log/choki-dar.log
sudo cp ~/chokidar/setup/choki-dar /etc/init.d/.
sudo chmod +x /etc/init.d/choki-dar
sudo update-rc.d choki-dar defaults

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
git commit -am "updated logic"
git push

