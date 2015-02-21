
sudo touch /var/log/choki-dar.log
sudo chown ${USER} /var/log/choki-dar.log
sudo cp ~/chokidar/setup/choki-dar /etc/init.d/.
sudo chmod +x /etc/init.d/choki-dar
sudo update-rc.d choki-dar defaults
