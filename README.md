# iNav Flight Control Web Server

Installing and starting the server:

-- we are looking for VDS, VPS or something similar, find out the IP address and whether it is possible to use web sockets on the server.

cloning the repository and running install.sh:

Install GIT:
sudo apt-get update
sudo apt-get install git

Clone a repository:
git clone https://github.com/zDimaBY/iNavFlightControlWebServer.git

Go to the repository directory:
cd iNavFlightControlWebServer/

Run install.sh:
sudo ./install.sh

Go to the IP of your web server through a web browser, write down the IP and port.
We launch a web socket through the console. Run the command: php /var/www/html/php/server.php

Through the equipment, we switch to GSM and control the flight on the website.

Встановлення та запуск сервера:

-- шукаємо VDS, VPS чи шось подібне, взнаєм ІП адресу та чи можливо використовувати на сервері веб сокети.

клонування репозиторію та запуск install.sh:

Встановити GIT:
sudo apt-get update
sudo apt-get install git

Клонувати репозиторій:
git clone https://github.com/zDimaBY/iNavFlightControlWebServer.git

Перейти до каталогу репозиторію:
cd iNavFlightControlWebServer/

Запустити install.sh:
sudo ./install.sh

Заходим через веб браузер на ІП вашого веб сервера, записуєм IP та порт.
Через консоль запускаєм веб сокет. Водим команду: php /var/www/html/php/server.php

Через апаратуру переходим у GSM та на вебсайті керуєм польотом.

![Image alt](https://github.com/zDimaBY/iNavFlightControlWebServer/blob/main/img/Readme/screenshot.png)
