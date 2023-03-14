<?php
$channelsJson = json_decode(file_get_contents('../json/channels.json'));
$channelsJson->channels = $_POST['channelsAJAX'];
file_put_contents('../json/channels.json', json_encode($channelsJson));
?>