let requests = [];
let Channel = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
let lat = "55.0000", lon = "30.00000";
let prevLat = null, prevLon = null;
let dataj;

let map = L.map('map').setView([lat, lon], 16); //Центер перегляду карти
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
  id: 'MapID',
  attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> вкладники'
}).addTo(map);
L.marker([lat, lon]).addTo(map);

let latlngs = [];
let iCoordinates = 0;

document.onkeypress = function (evant) {
  ValCH();// функція для отримання значень каналів
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
    jDATA();
    $.ajax({
      url: './php/updateChannels.php',
      type: 'POST',
      data: {
        channelsAJAX: dataJson,
      },
      cache: false,
    });
  }
};


function ValCH() {// Оголошуємо функції для отримання значень каналів, оновлення відображення та відправки AJAX-запиту
  for (let i = 0; i < 15; i++) {
    let ValueCH = 'ValueCH_' + i;
    let rangeValueCH = 'rangeValueCH_' + i;
    Channel[i] = document.getElementById(ValueCH);
    document.getElementById(rangeValueCH).innerHTML = Channel[i].value;
  }
}
function jDATA() {// Отримуємо значення всіх каналів та формуємо рядок з цими значеннями, розділяючи їх двокрапкою
  const channelValues = Array.from(document.querySelectorAll('[id^=ValueCH_]')).map(channel => channel.value).join(':');
  dataJson = channelValues;
}
function CH() {
  ValCH();
  jDATA();
  $.ajax({
    url: './php/updateChannels.php',
    type: 'POST',
    data: { channelsAJAX: dataJson }, 
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
          $(content).html(remap(data, 2, 30, -109, -53));
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
        alert(textStatus . errorThrown); // виводимо текст помилки
      }
    });
  });
})