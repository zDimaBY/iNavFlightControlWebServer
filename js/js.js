let Channel = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
let lat = "49.993634", lon = "37.811870"
let dataj;

let map = L.map('map').setView([lat, lon], 16); //Центер перегляду карти
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
  attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
}).addTo(map);
L.marker([lat, lon]).addTo(map);

let latlngs = [];
let iCoordinates = 0;

document.onkeypress = function (evant) {
  ValCH()
  if (evant.code == 'KeyA') {
    Channel[0].value = Channel[0].value - 1;
    document.getElementById('rangeValueCH_0').innerHTML = Channel[0].value;
  }
  if (evant.code == 'KeyD') {
    Channel[0].value = parseInt(Channel[0].value, 10) + 1;
    document.getElementById('rangeValueCH_0').innerHTML = Channel[0].value;
  }
  if (evant.code == 'KeyW') {
    Channel[1].value = parseInt(Channel[1].value, 10) + 1;
    document.getElementById('rangeValueCH_1').innerHTML = Channel[1].value;
  }
  if (evant.code == 'KeyS') {
    Channel[1].value = Channel[1].value - 1;
    document.getElementById('rangeValueCH_1').innerHTML = Channel[1].value;
  }
  if (evant.code == 'KeyQ') {
    Channel[3].value = Channel[3].value - 1;
    document.getElementById('rangeValueCH_3').innerHTML = Channel[3].value;
  }
  if (evant.code == 'KeyE') {
    Channel[3].value = parseInt(Channel[3].value, 10) + 1;
    document.getElementById('rangeValueCH_3').innerHTML = Channel[3].value;
  }
  if (evant.code == 'KeyZ') {
    Channel[2].value = Channel[2].value - 1;
    document.getElementById('rangeValueCH_2').innerHTML = Channel[2].value;
  }
  if (evant.code == 'KeyX') {
    Channel[2].value = parseInt(Channel[2].value, 10) + 1;
    document.getElementById('rangeValueCH_2').innerHTML = Channel[2].value;
  }
  jDATA()
  $.ajax({
    //type: 'GET',// тип запиту
    url: './php/upInDATA.php', // надсилаємо запит на сторінку upInDATA.php
    data: {
      dataJSON: dataj,
    }, //дані, які будуть передані із запитом
    cache: false,
  })
}

function CH() {
  ValCH()
  jDATA()
  $.ajax({
    //type: 'GET',// тип запиту
    url: './php/upInDATA.php', // надсилаємо запит на сторінку upInDATA.php
    data: {
      dataJSON: dataj,
    }, //дані, які будуть передані із запитом
    cache: false,
  })
}
function ValCH() {
  for (let i = 0; i < 15; i++) { //https://jsfiddle.net/zDimaBY/j9o1skgh/
    let ValueCH = 'ValueCH_' + i;
    let rangeValueCH = 'rangeValueCH_' + i;
    Channel[i] = document.getElementById(ValueCH);
    document.getElementById(rangeValueCH).innerHTML = Channel[i].value;
  }
}
function jDATA() {
  for (var i = 0; i < Channel.length; i++) {
    if (i == 0) {
      dataj = Channel[i].value;
    } else {
      dataj = dataj + ':' + Channel[i].value;
    }
  }
}
function show() {
  for (let i = 0; i < 16; i++) {
    let content = '#content-' + i;
    $.ajax({
      //type: 'GET',// тип запиту
      url: './php/upOutDATA.php', // надсилаємо запит на сторінку upOutDATA.php
      data: {
        ID: i,
      }, //дані, які будуть передані із запитом
      cache: false,
      success: function (data) {
        $(content).html(data)
        if (i == 3) {
          lat = data.substr(0, 2) + "." + data.substr(2, 8);
        }
        if (i == 4) {
          lon = data.substr(0, 2) + "." + data.substr(2, 8);
        }
      },
    })
  }

}

function mapLoop() {
  console.log(lat);
  latlngs[iCoordinates++] = [lat, lon];
  L.polyline(latlngs, { color: 'red' }).addTo(map);//Оновити шлях маркер
  if (iCoordinates > 100) {
    iCoordinates = 0;
  }
}

$(document).ready(function () {
  show();
  setInterval('show()', 1000);
  mapLoop();
  setInterval('mapLoop()', 10000);
})