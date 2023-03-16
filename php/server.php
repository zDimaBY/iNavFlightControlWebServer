<?php
error_reporting(E_ALL);
set_time_limit(0);/* Дозволяє скрипту очікувати з'єднання нескінченно. */
ob_implicit_flush();/* Включає приховане очищення виводу так, що ми бачимо дані. Як тільки вони з'являються. */

$jsonString = file_get_contents(dirname(__DIR__) . '/json/settings.json'); // Отримуємо вміст файлу settings.json
$data = json_decode($jsonString, true); // Декодуємо JSON-об'єкт у асоціативний масив
$address = $data['ip']; // Записуємо значення "ip" та "port" у змінні
$port = $data['port'];
$countPacket = 0;
if (($sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) === false) { // створення сокету, якщо ні то ...
    echo 'Неможливо виконати socket_create(): причина: ' . socket_strerror(socket_last_error()) . "\n";
}

if (socket_bind($sock, $address, $port) === false) {
    $lastError = socket_last_error($sock);
    if ($lastError == SOCKET_EADDRINUSE) { // SOCKET_EADDRINUSE константа помилки "Address already in use"
        $command = "fuser -n tcp -k " . (int) $port; // оновлення команди з числовим аргументом
        $output = shell_exec($command); // запуск команди
        if ($output === null) {
            echo 'Помилка виконання команди: ' . $command . "\n";
        } else {
            echo 'Зупинено процес за портом ' . $port . "\n";
            if (socket_bind($sock, $address, $port) === false) {
                echo 'Неможливо виконати socket_bind(): причина: ' . socket_strerror(socket_last_error($sock)) . "\n";
            }
        }
    } else {
        echo 'Неможливо виконати socket_bind(): причина: ' . socket_strerror($lastError) . "\n";
    }
}

if (socket_listen($sock, 5) === false) {
    echo 'Неможливо виконати socket_listen(): причина: ' . socket_strerror(socket_last_error($sock)) . "\n";
}

if (($msgsock = socket_accept($sock)) === false) { // Прийняття з'єднання через сокет
    echo 'Неможливо виконати socket_accept(): причина:' . socket_strerror(socket_last_error($sock)) . "\n";
}
while (true) {
    $channelsJson = json_decode(file_get_contents(dirname(__DIR__) . '/json/channels.json'));
    if (!$channelsJson) { // Виконуємо запис json з дефолтними даними у випадку помилки
        file_put_contents(dirname(__DIR__) . '/json/channels.json', '{"channels":"50:0:50:50:0:0:0:0:0:0:0:0:0:0:0"}');
        $channelsJson = json_decode(file_get_contents(dirname(__DIR__) . '/json/channels.json'));
    }
    $telemetryJson = json_decode(file_get_contents(dirname(__DIR__) . '/json/telemetry.json'));
    if (!$telemetryJson) { // Виконуємо запис json з дефолтними даними у випадку помилки
        file_put_contents(dirname(__DIR__) . '/json/telemetry.json', '{"telemetry":"Помилка:sever:PHP:0:0:0:0:0:0:0:0:0:0:0:0"}');
        $telemetryJson = json_decode(file_get_contents(dirname(__DIR__) . '/json/telemetry.json'));
    }
    if (false === ($buf = socket_read($msgsock, 1024, PHP_NORMAL_READ))) {
        echo 'Неможливо виконати socket_read(): причина: ' . socket_strerror(socket_last_error($msgsock)) . "\n";
        break;
    }
    if (!($buf = trim($buf))) {
        continue;
    }
    if ($buf == 'вихід') {
        break;
    }
    if ($buf == 'вимкнення') {
        socket_close($msgsock);
        break;
    }

    $telemetryJson->telemetry = $buf;
    
    $channelsArray = explode(':', $channelsJson->channels); //Підгтовка пакета для відправки
    $crc = crc16($channelsArray, count($channelsArray));//Створюєм СRC з масиву каналів
    $talkback = ":$channelsJson->channels:$crc\nch\0";//Формуємо повідомлення отримувачу
    
    $time_start = microtime(true);
    $elapsed_time_ms = sprintf("%.3f", ($time_start - $time_end) * 1000);
    echo "$elapsed_time_ms мс.\n"; //телеметрія $buf Канали $talkback

    socket_write($msgsock, $talkback, strlen($talkback));
    $time_end = microtime(true);

    if (!file_put_contents(dirname(__DIR__) . '/json/telemetry.json', json_encode($telemetryJson))) {
        echo 'Помилка запису даних у json-файл з телеметрією.' . "\n";
    }

    if ((microtime(true) - $timeout) >= 1) {
        $timeout = microtime(true);
        echo "$countPacket пакетів на 1с\n";
        $countPacket = 0;
    }
    $countPacket++;
};
socket_close($msgsock);
socket_close($sock);

function crc16($buffer, $size) {
    $crc = 0;
    for ($i = 0; $i < $size; $i++) {
        $data = $buffer[$i];
        for ($j = 16; $j > 0; $j--) {
            $crc = (($crc ^ $data) & 1) ? ($crc >> 1) ^ 0x8C : ($crc >> 1);
            $data >>= 1;
        }
    }
    return $crc;
}