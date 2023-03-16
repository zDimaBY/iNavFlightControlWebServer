sudo apt-get update && sudo apt-get install apache2 php && \
sudo apt-get install -y curl gnupg2 && sudo apt-get install -y curl gnupg2 && curl -sL https://deb.nodesource.com/setup_18.x | sudo -E bash - && sudo apt-get install -y nodejs && \ #Node.js
sudo sed -i 's/DirectoryIndex.*/DirectoryIndex index.php index.html index.cgi index.pl index.xhtml index.htm/g' /etc/apache2/mods-enabled/dir.conf && \
sudo touch /etc/apache2/sites-available/iNavFlightControlWebServer.conf && \
sudo echo -e "<VirtualHost *:80>\n\tServerAdmin webmaster@localhost\n\tDocumentRoot /var/www/iNavFlightControlWebServer\n\tServerName localhost\n\t<Directory /var/www/iNavFlightControlWebServer>\n\t\tOptions Indexes FollowSymLinks\n\t\tAllowOverride All\n\t\tRequire all granted\n\t</Directory>\n\tErrorLog ${APACHE_LOG_DIR}/error.log\n\tCustomLog ${APACHE_LOG_DIR}/access.log combined\n</VirtualHost>" | sudo tee /etc/apache2/sites-available/iNavFlightControlWebServer.conf && \
sudo a2dissite 000-default.conf && \
sudo mkdir /var/www/iNavFlightControlWebServer && sudo a2ensite iNavFlightControlWebServer.conf && \
sudo mv * /var/www/iNavFlightControlWebServer && rm -r /root/WEB_SERVER && rm /var/www/iNavFlightControlWebServer/install.sh && \
sudo chown -R www-data:www-data /var/www/iNavFlightControlWebServer && \
sudo chmod -R 777 /var/www/iNavFlightControlWebServer && \
sudo systemctl restart apache2 && \
sudo apache2ctl -S