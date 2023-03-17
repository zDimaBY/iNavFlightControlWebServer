let requests = [];
let Channel = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
let lat = "0", lon = "0", prevLat = null, prevLon = null;
let latlngs = [];
let allValueChannels;

let map = L.map('map').setView([lat, lon], 16); //Центер перегляду карти
L.marker([lat, lon]).addTo(map);// Маркер цетру карти перегляду

const osmUrl = 'https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png';
const osmAttrib = '© <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>';
const googleHybridUrl = 'https://mt1.google.com/vt/lyrs=y&x={x}&y={y}&z={z}';
const googleHybridAttrib = 'Google Hybrid';

const osmLayer = L.tileLayer(osmUrl, { id: 'MapID', attribution: osmAttrib });
const googleHybridLayer = L.tileLayer(googleHybridUrl, { attribution: googleHybridAttrib, maxZoom: 20 }).addTo(map); // Дадаєм обєкти слоїв .addTo(map)
const layersControl = L.control.layers({ 'OpenStreetMap': osmLayer, 'Google Hybrid': googleHybridLayer }, null, { collapsed: false }).addTo(map); // Додаєм у перемикач слої
googleHybridLayer.addTo(map); // Щоб встановити Google Hybrid шар за замовчуванням

document.onkeypress = function (evant) {
  inputToSpan();// функція для отримання значень каналів
  let channelIndex = -1;// Ініціалізуємо змінну для зберігання індексу повзунка
  switch (evant.code) {// Використовуємо switch для опрацювання різних кодів клавіш
    case 'KeyA':
      channelIndex = 3;
      Channel[channelIndex].value--;
      break;
    case 'KeyD':
      channelIndex = 3;
      Channel[channelIndex].value++;
      break;
    case 'KeyW':
      channelIndex = 2;
      Channel[channelIndex].value++;
      break;
    case 'KeyS':
      channelIndex = 2;
      Channel[channelIndex].value--;
      break;
    case 'KeyQ':
      channelIndex = 1;
      Channel[channelIndex].value--;
      break;
    case 'KeyE':
      channelIndex = 1;
      Channel[channelIndex].value++;
      break;
    case 'KeyZ':
      channelIndex = 0;
      Channel[channelIndex].value--;
      break;
    case 'KeyX':
      channelIndex = 0;
      Channel[channelIndex].value++;
      break;
    default:
      return;// Якщо код клавіші не співпадає з вищевказаними, повертаємося
  }
  if (channelIndex !== -1) {// Якщо був змінений канал, то оновлюємо та відправляємо AJAX-запит
    document.getElementById(`rangeValueCH_${channelIndex}`).innerHTML = Channel[channelIndex].value;
    allValueChannelsInput();
    $.ajax({
      url: './php/updateChannels.php',
      type: 'POST',
      data: {
        channelsAJAX: allValueChannels,
      },
      cache: false,
    });
  }
};


function inputToSpan() {// Оголошуємо функції для отримання значень каналів
  for (let i = 0; i < Channel.length; i++) {
    Channel[i] = document.getElementById('ValueCH_' + i);
    document.getElementById('rangeValueCH_' + i).textContent = Channel[i].value;
  }
}
function allValueChannelsInput() {// Отримуємо значення всіх каналів та формуємо рядок з цими значеннями, розділяючи їх двокрапкою
  allValueChannels = Array.from(document.querySelectorAll('[id^=ValueCH_]')).map(channel => channel.value).join(':');
}
function inputChannels() {//Виклик з index.php
  inputToSpan();
  allValueChannelsInput();
  $.ajax({//Відправка AJAX-запиту на оновлення каналів у файлі JSON
    url: './php/updateChannels.php',
    type: 'POST',
    data: { channelsAJAX: allValueChannels },
    cache: false,
  });
}
function remap(value, oldMin, oldMax, newMin, newMax) {
  return ((value - oldMin) / (oldMax - oldMin)) * (newMax - newMin) + newMin;
}
function show() {
  for (let i = 0; i < 15; i++) {
    let content = '#content-' + i;
    requests.push($.ajax({
      url: './php/updateTelemetry.php',
      data: { ID: i },
      cache: false
    }).done(function (data) {
      if ($(content).html() !== data) {//Провірка якщо дані змінились
        if (i === 14) {
          if (data == 99) {
            $(content).html("not known ").css('color', 'red');
          } else {
            let signal = Math.round((data / 31) * (-52 - (-115)) + (-115));// remap(data, 0, 31, -52, -115);
            if (signal >= -73) {
              $(content).html(signal).css('color', 'green');
            } else if (signal >= -83) {
              $(content).html(signal).css('color', 'darkgreen');
            } else if (signal >= -93) {
              $(content).html(signal).css('color', 'yellow');
            } else {
              $(content).html(signal).css('color', 'red');
            }
          }
        } else {
          $(content).html(data);
        }
        if (i === 3) {
          lat = data.substr(0, 2) + '.' + data.substr(2, 8);
        }
        if (i === 4) {
          lon = data.substr(0, 2) + '.' + data.substr(2, 8);
        }
      }
    }));
  }
  $.when.apply($, requests).done(function () {
    console.log('All requests completed');
  });
}
function drawRoute() {
  if (latlngs.length >= 100) {// Перевіряємо, чи кількість точок перевищує 100
    latlngs.shift();// Якщо так, то видаляємо першу точку
  }
  if (lat !== prevLat || lon !== prevLon) {
    latlngs.push([lat, lon]);// Додаємо нову точку
    L.polyline(latlngs, { color: 'red' }).addTo(map);// Оновлюємо маршрут на карті
    map.setView([lat, lon]);
    console.log('New point');
  }
  prevLat = lat;
  prevLon = lon;
}
$(document).ready(function () {
  show();
  setInterval('show()', 1000);
  drawRoute();
  setInterval('drawRoute()', 10000);
  $("#settings-form").submit(function (event) {
    event.preventDefault();
    var ip = $("#ip").val();
    var port = $("#port").val();
    $.ajax({
      url: "../php/saveSettingsJson.php",
      type: "POST",
      data: { ip: ip, port: port },
      success: function (data) {
        console.log("Налаштування збережені у файлі settings.json");
      }
    });
  });
  $("#btn").click(function () {
    $.ajax({
      url: "../php/saveSettingsJson.php", // шлях до PHP файлу
      type: "POST", // метод передачі даних
      dataType: "html", // тип даних, які отримаємо назад
      success: function (data) { // коли запит успішно завершено
        console.log(data); // виводимо дані, які повернув PHP файл
        console.log("Сервер стоп");
      },
      error: function (jqXHR, textStatus, errorThrown) { // якщо сталась помилка
        alert(textStatus.errorThrown); // виводимо текст помилки
      }
    });
  });
})