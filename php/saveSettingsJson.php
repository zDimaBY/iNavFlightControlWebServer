<?php
$ip = $_POST['ip'];
$port = $_POST['port'];
$settings = array('ip' => $ip, 'port' => $port);
$json = json_encode($settings);
file_put_contents('../json/settings.json', $json);
//error_reporting(E_ALL);
//echo shell_exec("php /var/www/iNavFlightControlWebServer/php/server.php"); // запуск server.php
?>