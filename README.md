# iNav Flight Control Web Server

Installing and starting the server:

1. We copy the files to the host. In my case /var/www/html ..
2. Find out the IP address of the server and whether it is possible to use web sockets on the server.
3. Assign the server IP in the file /var/www/html/php/server.php $address = 'Your IP'
$port = your free port; //For example 10000
4. Save the file.
5. We launch the web socket through the console. Run the command: php /var/www/html/php/server.php
6. We access your web server's IP through a web browser and control the flight

Встановлення та запуск сервера:

1. Копіюємо файли на хостин. В моєму випадку /var/www/html ..
2. Взнаєм ІП адресу сервера та чи можливо використовувати на сервері веб сокети.
3. Приписуєму ІП сервера в файлі /var/www/html/php/server.php $address = 'Ваш ІП'
$port = ваш вільний порт; //Наприклад 10000
4. Зберігаємо файл.
5. Через консоль запускаєм веб сокет. Водим команду: php /var/www/html/php/server.php
6. Заходим через веб браузер на ІП вашого веб сервера та керуєм польотом

![Image alt](https://github.com/zDimaBY/iNavFlightControlWebServer/blob/main/img/Readme/screenshot.png)
