/**
 * Class to handle Plantower PMS7003 sensor.
 * Author: liuzikai
 * Date: June 2018
 */ 

#include "PMS7003.h"

HardwareSerial PMS7003::pmsSerial(PMS7003::uart_port);

uint16_t PMS7003::input_checksum;

uint16_t PMS7003::pm1_0_cf1;
uint16_t PMS7003::pm2_5_cf1;
uint16_t PMS7003::pm10_0_cf1;
uint16_t PMS7003::pm1_0_amb;
uint16_t PMS7003::pm2_5_amb;
uint16_t PMS7003::pm10_0_amb;
uint16_t PMS7003::pm0_3_raw;
uint16_t PMS7003::pm0_5_raw;
uint16_t PMS7003::pm1_0_raw;
uint16_t PMS7003::pm2_5_raw;
uint16_t PMS7003::pm5_0_raw;
uint16_t PMS7003::pm10_0_raw;

uint8_t  PMS7003::version;
uint8_t  PMS7003::errorCode;

bool PMS7003::_enabled = false;

// Wait until the information is fully received
bool PMS7003::data_available(void) {
    return (pmsSerial.available() >= 32);
}

/* Helper function to retrive data from serial and add checkSum */

inline uint8_t PMS7003::read_uint8(void) {
    int inputLow = pmsSerial.read();
    input_checksum += inputLow;
    return inputLow;
}

inline uint16_t PMS7003::read_uint16(void) {
    int inputHigh = pmsSerial.read();
    int inputLow = pmsSerial.read();
    input_checksum += inputHigh + inputLow;
    return inputLow + (inputHigh << 8);
}

void PMS7003::begin(void) {
    pmsSerial.begin(9600, SERIAL_8N1, rx_pin, tx_pin);
    pinMode(set_pin, OUTPUT);
    set_enabled(false);
}

void PMS7003::set_enabled(bool enabled) {
    digitalWrite(set_pin, enabled);
}

bool PMS7003::update(void) {
    
    input_checksum = 0;
    
    // Data header
    if (read_uint8() != 0x42) return false;
    if (read_uint8() != 0x4D) return false;

    // Data lenth
    if (read_uint8() != 0x00) return false;
    if (read_uint8()!= 0x1c) return false;

    // store result into temporary veriables

    uint16_t _pm1_0_cf1 = read_uint16();
    uint16_t _pm2_5_cf1 = read_uint16();
    uint16_t _pm10_0_cf1 = read_uint16();

    uint16_t _pm1_0_amb = read_uint16();
    uint16_t _pm2_5_amb = read_uint16();
    uint16_t _pm10_0_amb = read_uint16();

    uint16_t _pm0_3_raw = read_uint16();
    uint16_t _pm0_5_raw = read_uint16();
    uint16_t _pm1_0_raw = read_uint16();
    uint16_t _pm2_5_raw = read_uint16();
    uint16_t _pm5_0_raw = read_uint16();
    uint16_t _pm10_0_raw = read_uint16();

    uint16_t _version = read_uint8();
    uint16_t _errorCode = read_uint8();

    // Read indivually, to avoid adding to checksum
    int inputHigh = pmsSerial.read();
    int inputLow = pmsSerial.read();
    uint16_t checksum = inputLow + (inputHigh << 8);

    if (checksum != input_checksum) return false;

    // Put data into interface variables

    pm1_0_cf1 = _pm1_0_cf1;
    pm2_5_cf1 = _pm2_5_cf1;
    pm10_0_cf1 = _pm10_0_cf1;

    pm1_0_amb = _pm1_0_amb;
    pm2_5_amb = _pm2_5_amb;
    pm10_0_amb = _pm10_0_amb;

    pm0_3_raw = _pm0_3_raw;
    pm0_5_raw = _pm0_5_raw;
    pm1_0_raw = _pm1_0_raw;
    pm2_5_raw = _pm2_5_raw;
    pm5_0_raw = _pm5_0_raw;
    pm10_0_raw = _pm10_0_raw;

    return true;
}
