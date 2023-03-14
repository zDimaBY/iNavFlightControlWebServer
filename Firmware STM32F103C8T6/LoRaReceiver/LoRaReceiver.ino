/*
  Скетч використовує 24256 байтів (37%) місця зберігання для програм. Межа 65536 байтів.
  Глобальні змінні використовують 2888 байтів (14%) динамічної пам’яті,  залишаючи 17592 байтів для локальних змінних. Межа 20480 байтів.
  
  Скетч використовує 26800 байтів (40%) місця зберігання для програм. Межа 65536 байтів.
  Глобальні змінні використовують 2324 байтів (11%) динамічної пам’яті,  залишаючи 18156 байтів для локальних змінних. Межа 20480 байтів.

  Скетч використовує 29376 байтів (44%) місця зберігання для програм. Межа 65536 байтів.
  Глобальні змінні використовують 2468 байтів (12%) динамічної пам’яті,  залишаючи 18012 байтів для локальних змінних. Межа 20480 байтів. 14.03.23

*/

#define FOR_i(from, to) for(int i = (from); i < (to); i++)

#include <SPI.h>
#include <LoRa_STM32.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>

HardwareSerial Serial3(USART3);
//HardwareSerial Serial2(USART2);
HardwareSerial Serial1(USART1);
SoftwareSerial LTM(PB8, PB9); // Встановлюємо наші піни Rx та Tx

//______________LORA
unsigned long timeoutRTH;//Таймеры: timeoutRTH - RTH//_______________Lora END

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
#define SBUS_UPDATE_RATE 15 //ms
uint8_t sbusPacket[SBUS_PACKET_LENGTH];
int rcChannels[SBUS_CHANNEL_NUMBER];
uint32_t sbusTime = 0;//_______________SBUS END

#define LIGHTTELEMETRY_START1 0x24 //$ __________________ LTM config
#define LIGHTTELEMETRY_START2 0x54 //T
#define LIGHTTELEMETRY_GFRAME 0x47 //G GPS + Baro altitude data ( Lat, Lon, Groundspeed, Alt, Sats, Sat fix)
#define LIGHTTELEMETRY_AFRAME 0x41 //A Attitude data ( Roll,Pitch, Heading )
#define LIGHTTELEMETRY_SFRAME 0x53 //S Sensors/Status data ( voltage, current, Rssi, Airspeed, Arm status, Failsafe status, Flight mode )
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
uint8_t    base_mode = 1;        // 1 = custom mode enabled, 4 = auto mode, 8 = guided mode, 16 = stabilize mode, 64 = manual mode, 128 = safety armed
uint32_t   custom_mode = 0;      // see mavlink dialect message enums. uses ardupilot plane custom mode set
uint8_t    system_state = 0;     // 0 = unknown, 3 = standby, 4 = active, 5 = critical
//    Параметри польоту LTM протоколу
int16_t    roll = 0;               // LTM [deg]
int16_t    pitch = 0;              // LTM [deg]
uint16_t   heading = 0;            // course over ground [degrees]
int32_t    lat = 0;
int32_t    lon = 0;
int32_t    alt = 0;                // LTM [m]
uint16_t   groundspeed = 0;        // LTM [m/s]
uint8_t    airspeed = 0;           // LTM [m/s]
uint8_t    gps_sats = 0;
uint8_t    fixType = 0;            // 0-1: no fix, 2: 2D fix, 3: 3D fix
uint16_t   voltage_battery = 0;    // [mV]
uint16_t   current_battery = 0;    // [mA]
uint8_t    rssi = 0;
uint8_t    armed = 0;
uint8_t    failsafe = 0;
// __________________ END LTM

//__________________ SIM800L config
#define APN "internet"
#define SERVER "193.169.195.19"
#define PORT "1000"

byte channelWeb[17] = {0}; //Канали з WEB Servera
char packetForModem[70]; // Оголошуємо pacetForModem Пакет даних каналів від сервера.
char telemetry[70];//Пакет даних телеметрії до сервера

byte rssiGsm;
bool gprsLoop;
unsigned long previousMillisCSQ;
//__________________ END SIM800L

unsigned long timeoutLora;//__________________ LORA config
struct RECEIVE_DATA {
  byte ch[16];
  byte crc; //байт crc
} data;// __________________ END LORA

void setup() {
  Serial1.begin(115200);// Діагностика:
  Serial2.begin(100000, SERIAL_8E2);//SBUS
  Serial3.begin(115200);//GSM800L
  LTM.begin(2400);//LTM
  while (!Serial1);// wait for module for connect.
  FOR_i(0, 15) {
    rcChannels[i] = 1500;
  }
  loraINIT();
}

void loop() {/*
  FOR_i(0, 15) {
    Serial1.print(rcChannels[i]);
    Serial1.print(" ");
  }
  Serial1.println(" LOOP");*/
  if (rcChannels[9] > 1900) {// якщо отримано сигнал з LORA модуля
    gprsINIT();
    gprsLoop = true;
    gprsLOOP(); // виконати gprsLOOP() і повернутися до початку циклу
    gprsCLOSE();
    return;
  }
  readLoraReceiver();
}
void RTH() {
  uint32_t currentMillis = millis();
  if (currentMillis > sbusTime) {
    if (millis() - timeoutRTH >= 2500) {
      sbusPreparePacket(sbusPacket, rcChannels, false, true);//sbusPreparePacket(sbusPacket, rcChannels, Встановлює, чи слід передавати прапор стійкості до відмов, Встановлює, чи слід передавати прапор втрати кадру)
      Serial2.write(sbusPacket, SBUS_PACKET_LENGTH);
    } else {
      sbusPreparePacket(sbusPacket, rcChannels, true, false);//sbusPreparePacket(sbusPacket, rcChannels, Встановлює, чи слід передавати прапор стійкості до відмов, Встановлює, чи слід передавати прапор втрати кадру)
      Serial2.write(sbusPacket, SBUS_PACKET_LENGTH);
    }
    sbusTime = currentMillis + SBUS_UPDATE_RATE;
  }
}
