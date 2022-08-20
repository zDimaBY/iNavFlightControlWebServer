#define FOR_i(from, to) for (int i = (from); i < (to); i++)

#include "itoa.h"
#include <GParser.h>    //https://github.com/GyverLibs/GParser
#include <GSMSimGPRS.h> //https://github.com/zDimaBY/GSMSim
#include <SPI.h>
#include <Scheduler.h>
#include <LoRa_STM32.h> //https://github.com/armtronix/arduino-LoRa-STM32

#define ledPin 5 //пін керуючий підсвічуванням

//______________LORA
unsigned long timeoutRTH; //Таймеры: timeoutRTH - RTH//_______________Lora END

#define RC_CHANNEL_MIN 990 //___________SBUS
#define RC_CHANNEL_MAX 2010
#define SBUS_MIN_OFFSET 173
#define SBUS_MID_OFFSET 992
#define SBUS_MAX_OFFSET 1811
#define SBUS_CHANNEL_NUMBER 16
#define SBUS_PACKET_LENGTH 25
#define SBUS_FRAME_HEADER 0x0f
#define SBUS_FRAME_FOOTER 0x00
#define SBUS_FRAME_FOOTER_V2 0x04
#define SBUS_STATE_FAILSAFE 0x08
#define SBUS_STATE_SIGNALLOSS 0x04
#define SBUS_UPDATE_RATE 15 // ms
uint8_t sbusPacket[SBUS_PACKET_LENGTH];
int rcChannels[SBUS_CHANNEL_NUMBER];
uint32_t sbusTime = 0; //_______________SBUS END

#define LIGHTTELEMETRY_START1 0x24 //$ __________________ LTM config
#define LIGHTTELEMETRY_START2 0x54 // T
#define LIGHTTELEMETRY_GFRAME 0x47 // G GPS + Baro altitude data ( Lat, Lon, Groundspeed, Alt, Sats, Sat fix)
#define LIGHTTELEMETRY_AFRAME 0x41 // A Attitude data ( Roll,Pitch, Heading )
#define LIGHTTELEMETRY_SFRAME 0x53 // S Sensors/Status data ( voltage, current, Rssi, Airspeed, Arm status, Failsafe status, Flight mode )
#define LIGHTTELEMETRY_GFRAMELENGTH 18
#define LIGHTTELEMETRY_AFRAMELENGTH 10
#define LIGHTTELEMETRY_SFRAMELENGTH 11
static uint8_t LTMserialBuffer[LIGHTTELEMETRY_GFRAMELENGTH - 4];
static uint8_t LTMreceiverIndex;
static uint8_t LTMcmd;
static uint8_t LTMrcvChecksum;
static uint8_t LTMreadIndex;
static uint8_t LTMframelength;
static uint8_t crlf_count = 0;
//    Системні параметри MAVLink див. у common.h для отримання додаткової інформації
uint8_t base_mode = 1;    // 1 = custom mode enabled, 4 = auto mode, 8 = guided mode, 16 = stabilize mode, 64 = manual mode, 128 = safety armed
uint32_t custom_mode = 0; // see mavlink dialect message enums. uses ardupilot plane custom mode set
uint8_t system_state = 0; // 0 = unknown, 3 = standby, 4 = active, 5 = critical
//    Параметри польоту LTM протоколу
int16_t roll = 0;     // LTM [deg] -> MAVLink [rad]
int16_t pitch = 0;    // LTM [deg] -> MAVLink [rad]
uint16_t heading = 0; // course over ground [degrees]
int32_t lat = 0;
int32_t lon = 0;
int32_t alt = 0;          // LTM [m]   -> MAVLink [cm]
uint16_t groundspeed = 0; // LTM [m/s] -> MAVLink [cm/s]
uint8_t airspeed = 0;     // LTM [m/s] -> MAVLink [cm/s]
uint8_t gps_sats = 0;
uint8_t fixType = 0;          // 0-1: no fix, 2: 2D fix, 3: 3D fix
uint16_t voltage_battery = 0; // [mV]
uint16_t current_battery = 0; // [mA]
uint8_t rssi = 0;
uint8_t armed = 0;
uint8_t failsafe = 0;
unsigned long previousTime_1 = 0;
unsigned long previousTime_2 = 0; // __________________ END LTM

#define APN "internet" //__________________ SIM800L config
#define SERVER "-----" //ІР тср сервера
#define PORT "10000"   //порт сервера
#define RESET_PIN 10   // Пін рестарту модема
GSMSimGPRS gsm(Serial3, RESET_PIN);
unsigned long timing, countSendLoop; // timing - зміна для timeOut, countSendLoop - тест
int ATtimeOut = 1000;                // timeOut через 1с AT команд
byte countSend, countSendTelem;      //лічильники пакетів
bool gprsInitBool, gprsClose;        //__________________ END SIM800L

char packet[70];     //Пакет даних каналів від сервера.
char telemetry[70];  //Пакет даних телеметрії до сервера.
char buff[15];       // Буфер телеметрії
byte channelWeb[17]; //Канали з WEB Servera

unsigned long timeoutLora; //__________________ LORA config
struct RECEIVE_DATA
{
  byte ch[16];
  byte crc; //байт crc
} data;     // __________________ END LORA

void setup()
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);              // Діагностика:
  Serial1.begin(100000, SERIAL_8E2); // SBUS
  Serial2.begin(2400);               // LTM
  Serial3.begin(115200);             // GSM800L
  Scheduler.startLoop(loop2);        //Запустимо 2 цикл
  while (!Serial3)
  {
    ; // wait for module for connect.
  }
  loraINIT();
}

void loop()
{ // run over and over
  if (data.ch[9] > 200)
  {
    if (!gprsInitBool)
    {
      gprsINIT();
      gprsInitBool = true;
      gprsClose = true;
    }
    gprsLOOP();
  }
  else
  {
    delay(500);
    if (gprsClose)
    {
      Serial3.write("AT+CIPSHUT\n");
      gprsClose = false;
      gprsInitBool = false;
    }
  }
}
void loop2()
{
  byte packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    Receive(packetSize);
  }
  else
  {
    delay(20);
  }
  RTH();
  yield();
}
void Receive(byte packetSize)
{
  LoRa.readBytes((uint8_t *)&data, packetSize);
  byte crc = crc16((byte *)&data, sizeof(data)); // Считуємо crc посилки повністю
  if (crc == 0)
  {
    if (data.ch[15] != 250)
    { // якщо одержувач не це пристрій або трансляція,
      // Serial.println("Надійшло повідомлення, але не для мене.");
      return; //пропустити решту функції
    }
    if (data.ch[9] >= 200)
    {
      return;
    }
    rcChannels[0] = map(data.ch[0], 255, 0, 1000, 2000);
    rcChannels[1] = map(data.ch[1], 255, 0, 1000, 2000);
    rcChannels[2] = map(data.ch[2], 0, 255, 1000, 2000);
    rcChannels[3] = map(data.ch[3], 255, 0, 1000, 2000);
    FOR_i(4, 14)
    {
      rcChannels[i] = map(data.ch[i], 0, 255, 1000, 2000);
    }
    if (data.ch[9] > 127)
    {
      digitalWrite(ledPin, HIGH);
    }
    else
    {
      digitalWrite(ledPin, LOW);
    }
    rcChannels[15] = map(LoRa.packetRssi(), -118, -34, 1000, 2000);
    sbusPreparePacket(sbusPacket, rcChannels, false, false);
    Serial1.write(sbusPacket, SBUS_PACKET_LENGTH);
    timeoutRTH = millis();
  }
}

void gprsLOOP()
{
  timing = millis();
  if (millis() - countSendLoop > 10000)
  {
    countSendLoop = millis();
    countSendTelem = countSend;
    countSend = 0;
  }
  ATLoop("AT+CIPSEND\n", ">"); //Відправити модему команду AT+CIPSEND для відправки даних та чикати відповідь > щоб сказати
  ltm_read();                  //Декодувати данні з польотного контролера.
  telemSend();                 //Зібрати дані в char telemetry[70]
  ATLoopSend(telemetry, "'");  //Відправити telemetry, та читати/парсити дані після символа '
}
void ATLoop(char *ATcmd, char *res)
{
  bool ATLoopWhile = true;
  Serial3.write(ATcmd);
  telemetry[0] = '\0'; // очистити
  while (ATLoopWhile)
  {
    if (Serial3.find(res))
    {
      ATLoopWhile = false;
    }
    if (millis() - timing > ATtimeOut)
    {
      ATLoopWhile = false;
    }
  }
}
void ATLoopSend(char *ATcmd, char *res)
{
  bool ATLoopWhile = true;
  byte iBuff = 0;
  Serial3.write(ATcmd);
  while (ATLoopWhile)
  {
    if (Serial3.available())
    {
      char ch = Serial3.read();
      if (ch == '\0')
      {
        packet[iBuff] = 0;
        iBuff = 0;
        packet[strlen(packet) - 2] = '\0';
        GParser data(strpbrk(packet, res) + 1, ':');
        int am = data.split();
        FOR_i(0, am)
        {
          rcChannels[i] = map(atol(data[i]), 0, 100, 1000, 2000);
        }
        Serial.println(atol(data[0]));
        sbusPreparePacket(sbusPacket, rcChannels, true, false);
        Serial1.write(sbusPacket, SBUS_PACKET_LENGTH);
        timeoutRTH = millis();
        ATLoopWhile = false;
        packet[0] = '\0'; // очистити
        countSend++;
        return;
      }
      else
      {
        packet[iBuff++] = ch;
      }
    }
    else
    {
      delay(10);
    }
    if (millis() - timing > ATtimeOut)
    {
      packet[0] = '\0'; // очистити
      ATLoopWhile = false;
      return;
    }
  }
}

void RTH()
{
  uint32_t currentMillis = millis();
  if (currentMillis > sbusTime)
  {
    if (millis() - timeoutRTH >= 2500)
    {
      sbusPreparePacket(sbusPacket, rcChannels, false, true); // sbusPreparePacket(sbusPacket, rcChannels, Встановлює, чи слід передавати прапор стійкості до відмов, Встановлює, чи слід передавати прапор втрати кадру)
      Serial1.write(sbusPacket, SBUS_PACKET_LENGTH);
    }
    else
    {
      sbusPreparePacket(sbusPacket, rcChannels, true, false); // sbusPreparePacket(sbusPacket, rcChannels, Встановлює, чи слід передавати прапор стійкості до відмов, Встановлює, чи слід передавати прапор втрати кадру)
      Serial1.write(sbusPacket, SBUS_PACKET_LENGTH);
    }
    sbusTime = currentMillis + SBUS_UPDATE_RATE;
  }
}

void loraINIT()
{
  LoRa.setPins(PA4, PB0, PA1); // SS, RST, DI0
  LoRa.begin(435E6);
  // LoRa.onReceive(onReceive); //register the receive callback
  LoRa.receive(); // put the radio into receive mode
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setTxPower(20);
  // LoRa.setCodingRate4(8);
  // LoRa.setSpreadingFactor(6);
  LoRa.setSyncWord(0x44);
}

void gprsINIT()
{
  gsm.init();
  gsm.gprsInit(APN);
  gsm.gprsInitServer(SERVER, PORT);
  gsm.connect();
  gsm.gprsServerConn();
  Serial.print("gsm.getIP() ");
  Serial.println(gsm.getIP());
}
void telemSend()
{
  strcat(telemetry, itoa(roll, buff, 10)); // 0 roll
  strcat(telemetry, ":");
  strcat(telemetry, itoa(pitch, buff, 10)); // 1 pitch
  strcat(telemetry, ":");
  strcat(telemetry, itoa(heading, buff, 10)); // 2 heading
  strcat(telemetry, ":");
  strcat(telemetry, ltoa(lat, buff, 10)); // 3 lat
  strcat(telemetry, ":");
  strcat(telemetry, ltoa(lon, buff, 10)); // 4 lon
  strcat(telemetry, ":");
  strcat(telemetry, ltoa((alt / 100), buff, 10)); // 5 (alt / 100)
  strcat(telemetry, ":");
  strcat(telemetry, ltoa(groundspeed, buff, 10)); // 6
  strcat(telemetry, ":");
  strcat(telemetry, itoa(gps_sats, buff, 10)); // 7
  strcat(telemetry, ":");
  strcat(telemetry, itoa(fixType, buff, 10)); // 8
  strcat(telemetry, ":");
  strcat(telemetry, ltoa(voltage_battery / 100, buff, 10)); // 9 (voltage_battery / 100)
  strcat(telemetry, ":");
  strcat(telemetry, ltoa(current_battery, buff, 10)); // 10 current_battery
  strcat(telemetry, ":");
  strcat(telemetry, itoa(rssi, buff, 10)); // 11
  strcat(telemetry, ":");
  strcat(telemetry, itoa(armed, buff, 10)); // 12
  strcat(telemetry, ":");
  strcat(telemetry, itoa(failsafe, buff, 10)); // 13
  strcat(telemetry, ":");
  strcat(telemetry, itoa(countSendTelem, buff, 10)); // 14
  strcat(telemetry, "\r\n\x1A");
}
//ФУНЦИЯ CRC шифрования ________________________________
byte crc16(byte *buffer, byte size)
{
  byte crc = 0;
  for (byte i = 0; i < size; i++)
  {
    byte data = buffer[i];
    for (int j = 16; j > 0; j--)
    {
      crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
      data >>= 1;
    }
  }
  return crc;
}

void sbusPreparePacket(uint8_t packet[], int channels[], bool isSignalLoss, bool isFailsafe)
{ // SBUS Protokol
  static int output[SBUS_CHANNEL_NUMBER] = {0};
  FOR_i(0, SBUS_CHANNEL_NUMBER)
  {
    output[i] = map(channels[i], RC_CHANNEL_MIN, RC_CHANNEL_MAX, SBUS_MIN_OFFSET, SBUS_MAX_OFFSET);
  }
  uint8_t stateByte = 0x00;
  if (isSignalLoss)
  {
    stateByte |= SBUS_STATE_SIGNALLOSS;
  }
  if (isFailsafe)
  {
    stateByte |= SBUS_STATE_FAILSAFE;
  }
  packet[0] = SBUS_FRAME_HEADER; // Header
  packet[1] = (uint8_t)(output[0] & 0x07FF);
  packet[2] = (uint8_t)((output[0] & 0x07FF) >> 8 | (output[1] & 0x07FF) << 3);
  packet[3] = (uint8_t)((output[1] & 0x07FF) >> 5 | (output[2] & 0x07FF) << 6);
  packet[4] = (uint8_t)((output[2] & 0x07FF) >> 2);
  packet[5] = (uint8_t)((output[2] & 0x07FF) >> 10 | (output[3] & 0x07FF) << 1);
  packet[6] = (uint8_t)((output[3] & 0x07FF) >> 7 | (output[4] & 0x07FF) << 4);
  packet[7] = (uint8_t)((output[4] & 0x07FF) >> 4 | (output[5] & 0x07FF) << 7);
  packet[8] = (uint8_t)((output[5] & 0x07FF) >> 1);
  packet[9] = (uint8_t)((output[5] & 0x07FF) >> 9 | (output[6] & 0x07FF) << 2);
  packet[10] = (uint8_t)((output[6] & 0x07FF) >> 6 | (output[7] & 0x07FF) << 5);
  packet[11] = (uint8_t)((output[7] & 0x07FF) >> 3);
  packet[12] = (uint8_t)((output[8] & 0x07FF));
  packet[13] = (uint8_t)((output[8] & 0x07FF) >> 8 | (output[9] & 0x07FF) << 3);
  packet[14] = (uint8_t)((output[9] & 0x07FF) >> 5 | (output[10] & 0x07FF) << 6);
  packet[15] = (uint8_t)((output[10] & 0x07FF) >> 2);
  packet[16] = (uint8_t)((output[10] & 0x07FF) >> 10 | (output[11] & 0x07FF) << 1);
  packet[17] = (uint8_t)((output[11] & 0x07FF) >> 7 | (output[12] & 0x07FF) << 4);
  packet[18] = (uint8_t)((output[12] & 0x07FF) >> 4 | (output[13] & 0x07FF) << 7);
  packet[19] = (uint8_t)((output[13] & 0x07FF) >> 1);
  packet[20] = (uint8_t)((output[13] & 0x07FF) >> 9 | (output[14] & 0x07FF) << 2);
  packet[21] = (uint8_t)((output[14] & 0x07FF) >> 6 | (output[15] & 0x07FF) << 5);
  packet[22] = (uint8_t)((output[15] & 0x07FF) >> 3);
  packet[23] = stateByte;         // Flags byte
  packet[24] = SBUS_FRAME_FOOTER; // Footer
}
void ltm_read()
{
  uint8_t c;
  static enum _serial_state {
    IDLE,
    HEADER_START1,
    HEADER_START2,
    HEADER_MSGTYPE,
    HEADER_DATA
  } c_state = IDLE;
  while (Serial2.available())
  {
    c = char(Serial2.read());
    if (c_state == IDLE)
    {
      c_state = (c == '$') ? HEADER_START1 : IDLE;
    }
    else if (c_state == HEADER_START1)
    {
      c_state = (c == 'T') ? HEADER_START2 : IDLE;
    }
    else if (c_state == HEADER_START2)
    {
      switch (c)
      {
      case 'G':
        LTMframelength = LIGHTTELEMETRY_GFRAMELENGTH;
        c_state = HEADER_MSGTYPE;
        break;

      case 'A':
        LTMframelength = LIGHTTELEMETRY_AFRAMELENGTH;
        c_state = HEADER_MSGTYPE;
        break;

      case 'S':
        LTMframelength = LIGHTTELEMETRY_SFRAMELENGTH;
        c_state = HEADER_MSGTYPE;
        break;

      default:
        c_state = IDLE;
      }
      LTMcmd = c;
      LTMreceiverIndex = 0;
    }
    else if (c_state == HEADER_MSGTYPE)
    {
      if (LTMreceiverIndex == 0)
      {
        LTMrcvChecksum = c;
      }
      else
      {
        LTMrcvChecksum ^= c;
      }

      if (LTMreceiverIndex == LTMframelength - 4)
      { // отриманий байт контрольної суми
        if (LTMrcvChecksum == 0)
        {
          ltm_check();
          c_state = IDLE;
        }
        else
        { // неправильна контрольна сума, скинути пакет
          c_state = IDLE;
        }
      }
      else
        LTMserialBuffer[LTMreceiverIndex++] = c;
    }
  }
}

void ltm_check()
{
  LTMreadIndex = 0;
  if (LTMcmd == LIGHTTELEMETRY_GFRAME)
  {
    lat = ltmread_u32();
    lon = ltmread_u32();
    groundspeed = (uint16_t)ltmread_u8();
    alt = ltmread_u32() * 100;
    uint8_t ltm_satsfix = ltmread_u8();
    gps_sats = (ltm_satsfix >> 2) & 0xFF;
    fixType = ltm_satsfix & 0b00000011;
  }
  if (LTMcmd == LIGHTTELEMETRY_AFRAME)
  {
    pitch = ltmread_u16();
    roll = ltmread_u16();
    heading = ltmread_u16();
  }
  if (LTMcmd == LIGHTTELEMETRY_SFRAME)
  {
    //    static int frametick = 0;
    voltage_battery = ltmread_u16();
    current_battery = ltmread_u16();
    rssi = ltmread_u8();
    airspeed = ltmread_u8();
    uint8_t ltm_armfsmode = ltmread_u8();
    armed = (ltm_armfsmode & 0b00000001);
    failsafe = (ltm_armfsmode >> 1) & 0b00000001;
    custom_mode = (ltm_armfsmode >> 2) & 0b00111111;
    if ((custom_mode == 0) || (custom_mode == 3) || (custom_mode == 4))
    {
      base_mode = 65;
    }
    else if ((custom_mode == 2) || (custom_mode == 5) || (custom_mode == 6) || (custom_mode == 7) || (custom_mode == 8))
    {
      base_mode = 17;
    }
    else if (10 <= custom_mode <= 15)
    {
      base_mode = 25;
    }
    if (armed == 1)
    {
      base_mode += 128;
      system_state = 4;
    }
    if (failsafe == 1)
    {
      system_state = 5;
    }
  }
}
uint8_t ltmread_u8()
{
  return LTMserialBuffer[LTMreadIndex++];
}
uint16_t ltmread_u16()
{
  uint16_t t = ltmread_u8();
  t |= (uint16_t)ltmread_u8() << 8;
  return t;
}
uint32_t ltmread_u32()
{
  uint32_t t = ltmread_u16();
  t |= (uint32_t)ltmread_u16() << 16;
  return t;
}
