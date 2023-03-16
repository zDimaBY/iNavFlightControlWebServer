const net = require('net');
const fs = require('fs');

const settings = JSON.parse(fs.readFileSync(__dirname + '/../json/settings.json'));
const address = settings.ip;
const port = settings.port;
let countPacket = 0;
let time_end = time_start = Date.now();
let buf = '';

const server = net.createServer((socket) => {
  console.log('Connected');

  socket.on('data', (data) => {
    buf += data.toString().trim();
    if (data.toString().endsWith('\n')) {
      channelsJson = JSON.parse(fs.readFileSync(__dirname + '/../json/channels.json'));
      const telemetry = {
        data: buf,
        toJSON() {
          return { telemetry: this.data };
        }
      };

      const crc = crc16(channelsJson.channels.split(':').map(Number));
      const talkback = `:${channelsJson.channels}:${crc}\nch\0`;

      time_start = Date.now();
      const elapsed_time_ms = ((time_start - time_end)).toFixed(0);
      console.log(`${elapsed_time_ms}мс. telemetry: ${buf} channels: ${talkback}`);//Повідомлення у консоль
      socket.write(talkback);
      time_end = Date.now();
      countPacket++;

      fs.writeFile(__dirname + '/../json/telemetry.json', JSON.stringify(telemetry), (err) => {
        if (err) throw err;
        console.log('Телеметрія успішно записана у файл!');
      });
      buf = '';
    }
  });

  socket.on('end', () => {
    console.log('Disconnected');
  });

  socket.on('error', (err) => {
    console.error(err);
  });
});

server.on('error', (err) => {
  console.error('Server error:', err);
});

server.listen(port, address, () => {
  console.log(`Server started at ${address}:${port}`);
});

setInterval(function () {
  console.log(`${countPacket} пакетів на 1с `);
  countPacket = 0;
}, 1000);

function crc16(buffer) {
  let crc = 0;
  for (let i = 0; i < buffer.length; i++) {
    let data = buffer[i];
    for (let j = 16; j > 0; j--) {
      crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
      data >>= 1;
    }
  }
  return crc;
}