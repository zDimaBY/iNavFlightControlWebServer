<!DOCTYPE html>
<html lang="ua">

<head>
    <meta charset="UTF-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>SERVER</title>
    <link rel="shortcut icon" href="./img/Dart.ico" type="image/x-icon">
    <script src="https://code.jquery.com/jquery-3.6.0.min.js" crossorigin="anonymous"></script>
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.8.0/dist/leaflet.css"
        integrity="sha512-hoalWLoI8r4UszCkZ5kL8vayOGVae1oxXe/2A4AO6J9+580uKHDO3JdHb7NzwwzK5xr/Fs0W40kiNHxM9vyTtQ=="
        crossorigin="" />
    <script src="https://unpkg.com/leaflet@1.8.0/dist/leaflet.js"
        integrity="sha512-BB3hKbKWOc9Ez/TAwyWxNXeoV9c1v6FIeYiBieIWkpLjauysF18NzgR1MBNBXf8/KABdlkX68nAhlwcDFLGPCQ=="
        crossorigin=""></script>



    <link rel="stylesheet" type="text/css" href="./css/style.css" />
</head>

<body>
    <div id="map"></div>
    <div class="position">
        <div class="CH_Control">
            <h3>Налаштування IP та порту</h3>
            <?php
            $jsonString = file_get_contents('./json/settings.json'); // Отримуємо вміст файлу settings.json
            $data = json_decode($jsonString, true); // Декодуємо JSON-об'єкт у асоціативний масив
            echo '<form id="settings-form">
            <label for="ip">IP адреса:</label>
            <input type="text" id="ip" name="ip" value="' . $data['ip'] . '">
            <label for="port">Порт:</label>
            <input type="text" id="port" name="port" value="' . $data['port'] . '">
            <button type="submit">Зберегти</button>
          </form>';// https://jsfiddle.net/zDimaBY/ntk6qow0/ - тест джойстика
            ?>
            <p class="r1">Управління польотом</p>
            <div class="boxFlex">
                <div>
                    <p class="">Roll [A]</p>
                    <div class="boxFlex">
                        <input class="styleRange rotateRange" type="range" min="0" max="99" id="ValueCH_3"
                            onchange="inputChannels();" list="rangeListCH_4" />
                        <span class="submitButtonRange submitButton" id="rangeValueCH_3">50</span>
                    </div>
                </div>
                <div>
                    <p class="">Pitch [E]</p>
                    <input class="styleRange" type="range" min="0" max="99" id="ValueCH_2" onchange="inputChannels();"
                        list="rangeListCH_3" />
                    <span class="submitButton" id="rangeValueCH_2">50</span>
                </div>
                <div>
                    <p class="">Throttle [T]</p>
                    <input class="styleRange" type="range" min="0" value="0" max="99" id="ValueCH_0" onchange="inputChannels();">
                    <span class="submitButton" id="rangeValueCH_0">0</span>
                </div>
                <div>
                    <p class="">Yaw [R]</p>
                    <div class="boxFlex">
                        <input class="styleRange rotateRange" type="range" min="0" max="99" id="ValueCH_1"
                            onchange="inputChannels();">
                        <span class="submitButtonRange submitButton" id="rangeValueCH_1">50</span>
                    </div>
                </div>
            </div>
            <div class="textalignLeft">
                <div>
                    CH 5<input class="styleRange CH_5" type="range" min="0" value="10" max="99" id="ValueCH_4"
                        onchange="inputChannels();" list="rangeListCH_5" />
                    <span class="submitButton" id="rangeValueCH_4">10</span>OSD Mod
                </div>
                <div>
                    CH 6<input class="styleRange CH_5" type="range" min="0" value="0" max="99" id="ValueCH_5"
                        onchange="inputChannels();" list="rangeListCH_6" />
                    <span class="submitButton" id="rangeValueCH_5">0</span>Flight Modes
                </div>
                <div>
                    CH 7<input class="styleRange CH_5" type="range" min="0" value="0" max="99" id="ValueCH_6"
                        onchange="inputChannels();" list="rangeListCH_7" />
                    <span class="submitButton" id="rangeValueCH_6">0</span>
                </div>
                <div>
                    CH 8<input class="styleRange CH_5" type="range" min="0" value="0" max="99" id="ValueCH_7"
                        onchange="inputChannels();" list="rangeListCH_8" />
                    <span class="submitButton" id="rangeValueCH_7">0</span>Arming 2.5-100%
                </div>
                <div>
                    CH 9<input class="styleRange CH_5" type="range" min="0" value="0" max="99" id="ValueCH_8"
                        onchange="inputChannels();" list="rangeListCH_9" />
                    <span class="submitButton" id="rangeValueCH_8">0</span>
                </div>
                <div>
                    CH 10<input class="styleRange CH_5" type="range" min="0" value="95" max="99" id="ValueCH_9"
                        onchange="inputChannels();" list="rangeListCH_10" />
                    <span class="submitButton" id="rangeValueCH_9">95</span>GSM ON 80-100%
                </div>
                <div>
                    CH 11<input class="styleRange CH_5" type="range" min="0" value="0" max="99" id="ValueCH_10"
                        onchange="inputChannels();" list="rangeListCH_11" />
                    <span class="submitButton" id="rangeValueCH_10">0</span>
                </div>
                <div>
                    CH 12<input class="styleRange CH_5" type="range" min="0" value="0" max="99" id="ValueCH_11"
                        onchange="inputChannels();" list="rangeListCH_12" />
                    <span class="submitButton" id="rangeValueCH_11">0</span>
                </div>
                <div>
                    CH 13<input class="styleRange CH_5" type="range" min="0" value="0" max="99" id="ValueCH_12"
                        onchange="inputChannels();" list="rangeListCH_13" />
                    <span class="submitButton" id="rangeValueCH_12">0</span>
                </div>
                <div>
                    CH 14<input class="styleRange CH_5" type="range" min="0" value="0" max="99" id="ValueCH_13"
                        onchange="inputChannels();" list="rangeListCH_14" />
                    <span class="submitButton" id="rangeValueCH_13">0</span>FAILSAFE 5-100%, cam 2 - 0-100%
                </div>
                <div>
                    CH 15<input class="styleRange CH_5" type="range" min="0" value="0" max="99" id="ValueCH_14"
                        onchange="inputChannels();" list="rangeListCH_15" />
                    <span class="submitButton" id="rangeValueCH_14">0</span>
                </div>
            </div>
        </div>
        <div class="boxFlex Telemetre">
            <div class="">
                <p class="r1">Телеметрія 1</p>
                <div class="r2">
                    <div class="r3" id="content-0"></div>
                    <div class="r3"> roll</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-1"></div>
                    <div class="r3"> pitch</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-2"></div>
                    <div class="r3"> heading</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-3"></div>
                    <div class="r3">lat</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-4"></div>
                    <div class="r3">lon</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-5"></div>
                    <div class="r3">m alt</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-6"></div>
                    <div class="r3">groundspeed</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-7"></div>
                    <div class="r3">gps_sats</div>
                </div>
            </div>

            <div class="">
                <p class="r1">Телеметрія 2</p>
                <div class="r2">
                    <div class="r3" id="content-8"></div>
                    <div class="r3">fixType</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-9"></div>
                    <div class="r3">voltage_battery</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-10"></div>
                    <div class="r3">current_battery</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-11"></div>
                    <div class="r3">rssi</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-12"></div>
                    <div class="r3">armed</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-13"></div>
                    <div class="r3">failsafe</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-14"></div>
                    <div class="r3"> RSSI dBm / 10с</div>
                </div>
                <div class="r2">
                    <div class="r3" id="content-15"></div>
                    <div class="r3"> x</div>
                </div>
            </div>
            <div class="">
                <button id="btn">Виконати PHP файл</button>
            </div>
        </div>
    </div>
</body>
<script src="./js/js.js"></script>

</html>