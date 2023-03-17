void loraINIT() {
  LoRa.begin(435E6);
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setTxPower(20);
  //LoRa.setCodingRate4(8);
  //LoRa.setSpreadingFactor(6);
  LoRa.setSyncWord(0x44);
  //LoRa.onReceive(onReceive); //register the receive callback
  LoRa.receive();// put the radio into receive mode
}
//_______________ CRC шифрування
byte crc16(byte *buffer, byte size) {
  byte crc = 0;
  for (byte i = 0; i < size; i++) {
    byte data = buffer[i];
    for (int j = 16; j > 0; j--) {
      crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
      data >>= 1;
    }
  }
  return crc;
}
void readLoraReceiver() {
  byte packetSize = LoRa.parsePacket(); // перевірка наявності пакетів в ефірі
  if (packetSize) { // якщо пакет отримано
    LoRa.readBytes((uint8_t *)&data, packetSize);
    byte crcValue = crc16((byte *)&data, sizeof(data)); // Считуємо crc посилки повністю
    if (crcValue == 0 && data.ch[15] == 250) { // якщо crc вірний і одержувач це пристрій або трансляція
      // отримано сигнал з приймача
      rcChannels[0] = map(data.ch[0], 255, 0, 1000, 2000);
      rcChannels[1] = map(data.ch[1], 255, 0, 1000, 2000);
      rcChannels[2] = map(data.ch[2], 0, 255, 1000, 2000);
      rcChannels[3] = map(data.ch[3], 255, 0, 1000, 2000);
      FOR_i(4, 14) {
        rcChannels[i] = map(data.ch[i], 0, 255, 1000, 2000);
      }
      rcChannels[15] = map(LoRa.packetRssi(), -118, -34, 1000, 2000);
      sbusPreparePacket(sbusPacket, rcChannels, false, false);
      Serial2.write(sbusPacket, SBUS_PACKET_LENGTH);
      timeoutRTH = millis();
    }
  } else { // якщо пакет не отримано
    RTH();
  }
}
