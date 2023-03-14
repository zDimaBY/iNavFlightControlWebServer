<?php
$telemetryJson = file_get_contents('../json/telemetry.json');
$content = explode(':', json_decode($telemetryJson)->telemetry);
echo $content[$_GET['ID']];
?>
