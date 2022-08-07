<?php
$inDATA = json_decode(file_get_contents('../json/inDATA.json'));
$inDATA->inDATA = $_GET['dataJSON'];
file_put_contents('../json/inDATA.json', json_encode($inDATA));