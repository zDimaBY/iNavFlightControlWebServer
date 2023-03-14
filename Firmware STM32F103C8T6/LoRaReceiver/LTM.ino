void ltm_read() {
  uint8_t c;
  static enum _serial_state {
    IDLE,
    HEADER_START1,
    HEADER_START2,
    HEADER_MSGTYPE,
    HEADER_DATA
  }
  c_state = IDLE;
  while (LTM.available()) {
    c = char(LTM.read());
    if (c_state == IDLE) {
      c_state = (c == '$') ? HEADER_START1 : IDLE;
    } else if (c_state == HEADER_START1) {
      c_state = (c == 'T') ? HEADER_START2 : IDLE;
    } else if (c_state == HEADER_START2) {
      switch (c) {
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
    } else if (c_state == HEADER_MSGTYPE) {
      if (LTMreceiverIndex == 0) {
        LTMrcvChecksum = c;
      } else {
        LTMrcvChecksum ^= c;
      }

      if (LTMreceiverIndex == LTMframelength - 4) { // отриманий байт контрольної суми
        if (LTMrcvChecksum == 0) {
          ltm_check();
          c_state = IDLE;
        } else {     // неправильна контрольна сума, скинути пакет
          c_state = IDLE;
        }
      } else LTMserialBuffer[LTMreceiverIndex++] = c;
    }
  }
}

void ltm_check() {
  LTMreadIndex = 0;
  if (LTMcmd == LIGHTTELEMETRY_GFRAME) {
    lat = ltmread_u32();
    lon = ltmread_u32();
    groundspeed = (uint16_t)ltmread_u8();
    alt = ltmread_u32() * 100;
    uint8_t ltm_satsfix = ltmread_u8();
    gps_sats = (ltm_satsfix >> 2) & 0xFF;
    fixType = ltm_satsfix & 0b00000011;
  }
  if (LTMcmd == LIGHTTELEMETRY_AFRAME) {
    pitch = ltmread_u16();
    roll = ltmread_u16();
    heading = ltmread_u16();
  }
  if (LTMcmd == LIGHTTELEMETRY_SFRAME) {
    //    static int frametick = 0;
    voltage_battery = ltmread_u16();
    current_battery = ltmread_u16();
    rssi = ltmread_u8();
    airspeed = ltmread_u8();
    uint8_t ltm_armfsmode = ltmread_u8();
    armed = (ltm_armfsmode & 0b00000001);
    failsafe = (ltm_armfsmode >> 1) & 0b00000001;
    custom_mode = (ltm_armfsmode >> 2) & 0b00111111;
    if ((custom_mode == 0) || (custom_mode == 3) || (custom_mode == 4)) {
      base_mode = 65;
    } else if ((custom_mode == 2) || (custom_mode == 5) || (custom_mode == 6) || (custom_mode == 7) || (custom_mode == 8)) {
      base_mode = 17;
    } else if (10 <= custom_mode <= 15) {
      base_mode = 25;
    }
    if (armed == 1) {
      base_mode += 128;
      system_state = 4;
    }
    if (failsafe == 1) {
      system_state = 5;
    }
  }
}
uint8_t ltmread_u8() {
  return LTMserialBuffer[LTMreadIndex++];
}
uint16_t ltmread_u16() {
  uint16_t t = ltmread_u8();
  t |= (uint16_t)ltmread_u8() << 8;
  return t;
}
uint32_t ltmread_u32() {
  uint32_t t = ltmread_u16();
  t |= (uint32_t)ltmread_u16() << 16;
  return t;
}
