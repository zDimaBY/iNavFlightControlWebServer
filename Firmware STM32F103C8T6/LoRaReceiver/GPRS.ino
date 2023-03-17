byte sendCommandAndWaitForResponse(const char* command, const char* searchStr, unsigned long timeout = 10000, Stream& SIM800L = Serial3) {// Атрибути: команда для відправи в модем, команда відповіді від модемо якщо успішно виконалась, тайм-аут якщо не дочикались відповіді, UART по якому спілкується модем.
  SIM800L.println(command);
  //Serial1.println(command);
  SIM800L.flush();
  byte iBuff = 0;
  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (SIM800L.available()) {
      char c = SIM800L.read();
      packetForModem[iBuff++] = c;
      //Serial1.write(c); // виводимо відповідь модема в Serial для зручності налагодження
      if (c == searchStr[0]) { // перевіряємо чи поточний символ співпадає з першим символом шуканого рядка
        bool found = true;
        unsigned long searchStart = millis();
        for (int i = 1; searchStr[i] != '\0'; i++) { // перевіряємо чи наступні символи співпадають з рештою шуканого рядка
          while (!SIM800L.available() && (millis() - searchStart < timeout)) {
            delay(1);
          }
          if (SIM800L.available()) {
            char nextChar = SIM800L.read();
            //Serial1.write(nextChar); // виводимо відповідь модема в Serial для зручності налагодження
            if (nextChar != searchStr[i]) {
              found = false;
              break;
            }
          }
        }

        if (found) {
          return 1; // якщо шуканий рядок був знайдений, повертаємося з функції
        }
      }
    }
  }
  return 0;
}
void parsePacketForModem(char* packetForModem, byte* channelWeb) {
  char* token = strtok(packetForModem, ":"); // розділяємо рядок за ":" та отримуємо перше значення
  byte i = 0;
  while (token != nullptr && i < 17) {
    channelWeb[i] = atoi(token); // перетворюємо отримане значення у ціле число та записуємо його у масив
    token = strtok(nullptr, ":"); // отримуємо наступне значення
    i++;
  }
}
int getSignalQuality(Stream& SIM800L = Serial3) {
  const unsigned long timeout = 100; // timeout in milliseconds
  unsigned long start_time = millis();
  SIM800L.println("AT+CSQ");
  SIM800L.flush();
  while (millis() - start_time < timeout) {
    if (SIM800L.available()) {
      String response = SIM800L.readStringUntil('\n');
      if (response.indexOf("+CSQ:") != -1) {
        byte comma_index = response.indexOf(",");
        rssiGsm = response.substring(response.indexOf(":") + 1, comma_index).toInt();
        byte ber = response.substring(comma_index + 1).toInt();
        return rssiGsm;
      }
    }
  }
  return -1;// Timeout occurred
}
char* creatingTelemetryMessage(char* telemetryMsg) {
  ltm_read();
  sprintf(telemetryMsg, "%d:%d:%d:%ld:%ld:%ld:%ld:%d:%d:%ld:%ld:%d:%d:%d:%d\r\n\x1A", roll, pitch, heading, lat, lon, (alt / 100), groundspeed, gps_sats, fixType, (voltage_battery / 100), current_battery, rssi, armed, failsafe, rssiGsm);
  return telemetryMsg;
}

void gprsINIT() {
  sendCommandAndWaitForResponse("AT", "OK");
  sendCommandAndWaitForResponse("AT+SAPBR=3,1,APN,"APN"", "OK");
  sendCommandAndWaitForResponse("AT+SAPBR=1,1", "OK");
  sendCommandAndWaitForResponse("AT+SAPBR=2,1", "1,1");
  sendCommandAndWaitForResponse("AT+CIPSTART=\"TCP\",\""SERVER"\",\""PORT"\"", "CONNECT OK");
}
void gprsLOOP() {
  while (gprsLoop) {
    RTH();
    if (millis() - previousMillisCSQ >= 10000) {
      previousMillisCSQ = millis();
      getSignalQuality();
    }
    sendCommandAndWaitForResponse("AT+CIPSEND", ">", 1000);
    sendCommandAndWaitForResponse(creatingTelemetryMessage(telemetry), "ch", 2000);
    parsePacketForModem(packetForModem, channelWeb);
    byte crcValue = crc16(&channelWeb[1], 16); // Считуємо crc посилки повністю
    if (crcValue == 0) { // якщо crc вірний
      FOR_i(0, 16) {//Запишем канали для SBUS
        rcChannels[i] = map(channelWeb[i + 1], 0, 100, 1000, 2000);
        //Serial1.print(rcChannels[i]);
        //Serial1.print(" ");
      }
      //Serial1.println(" GSM");
      if (rcChannels[9] < 1800) {
        gprsLoop = false;
      }
      timeoutRTH = millis();
      sbusPreparePacket(sbusPacket, rcChannels, false, false);
      Serial2.write(sbusPacket, SBUS_PACKET_LENGTH);
    }
  }
}
void gprsCLOSE() {
  sendCommandAndWaitForResponse("AT+CIPCLOSE", "CLOSED", 1000);
  sendCommandAndWaitForResponse("AT+CIPSHUT", "SHUT OK", 100);
}
