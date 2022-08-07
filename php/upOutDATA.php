<?php
$ourDataJSON = file_get_contents('../json/outDATA.json');
$content = explode(':', json_decode($ourDataJSON)->outDATA);
echo $content[$_GET['ID']];