<?php
error_reporting(E_ALL);

/* Дозволяє скрипту очікувати з'єднання нескінченно. */
set_time_limit(0);

/* Включає приховане очищення виводу так, що ми бачимо дані
 * Як тільки вони з'являються. */
ob_implicit_flush();

$address = 'Yor IP';
$port = 10000;

if (($sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) === false) {
    echo 'Неможливо виконати socket_create(): причина: ' .
        socket_strerror(socket_last_error()) .
        "\n";
}

if (socket_bind($sock, $address, $port) === false) {
    echo 'Неможливо виконати socket_bind(): причина: ' .
        socket_strerror(socket_last_error($sock)) .
        "\n";
}

if (socket_listen($sock, 5) === false) {
    echo 'Неможливо виконати socket_listen(): причина: ' .
        socket_strerror(socket_last_error($sock)) .
        "\n";
}

do {
    if (($msgsock = socket_accept($sock)) === false) {
        echo 'Неможливо виконати socket_accept(): причина:' .
            socket_strerror(socket_last_error($sock)) .
            "\n";
        break;
    }
    /* Надсилаємо інструкції. */
    $msg = " STAR\n";
    socket_write($msgsock, $msg, strlen($msg));
    do {
        $inDATA = json_decode(
            file_get_contents('/var/www/html/json/inDATA.json')
        );
        $outDATA = json_decode(
            file_get_contents('/var/www/html/json/outDATA.json')
        );
        if (false === ($buf = socket_read($msgsock, 1024, PHP_NORMAL_READ))) {
            echo 'Неможливо виконати socket_read(): причина: ' .
                socket_strerror(socket_last_error($msgsock)) .
                "\n";
            break 2;
        }
        if (!($buf = trim($buf))) {
            continue;
        }
        if ($buf == 'вихід') {
            break;
        }
        if ($buf == 'вимкнення') {
            socket_close($msgsock);
            break 2;
        }
        $outDATA->outDATA = $buf;
        $talkback = "'$inDATA->inDATA'.\0\n";
        socket_write($msgsock, $talkback, strlen($talkback));
        echo "$outDATA->outDATA\n";
        file_put_contents(
            '/var/www/html/json/outDATA.json',
            json_encode($outDATA)
        ); // Перекодувати у формат та записати у файл.
    } while (true);
    socket_close($msgsock);
} while (true);

socket_close($sock);