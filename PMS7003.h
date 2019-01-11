/**
 * Class to handle Plantower PMS7003 sensor.
 * Author: liuzikai
 * Date: June 2018
 */ 

#ifndef DSP_MONITOR_PMS7003_H_
#define DSP_MONITOR_PMS7003_H_

#include <Arduino.h>

class PMS7003 {

private:

    static HardwareSerial pmsSerial;

    /** Hardware configurations **/
    static constexpr int uart_port = 1;
    static constexpr int8_t rx_pin = 9;
    static constexpr int8_t tx_pin = 10;
    static constexpr int8_t set_pin = 5;

    static uint16_t input_checksum;

    inline static uint8_t read_uint8(void);
    inline static uint16_t read_uint16(void);

    static bool _enabled;

public:
    
    static bool data_available(void);

    static uint16_t pm1_0_cf1;
    static uint16_t pm2_5_cf1;
    static uint16_t pm10_0_cf1;
    static uint16_t pm1_0_amb;
    static uint16_t pm2_5_amb;
    static uint16_t pm10_0_amb;
    static uint16_t pm0_3_raw;
    static uint16_t pm0_5_raw;
    static uint16_t pm1_0_raw;
    static uint16_t pm2_5_raw;
    static uint16_t pm5_0_raw;
    static uint16_t pm10_0_raw;
    
    static uint8_t  version;
    static uint8_t  errorCode;

    static void begin(void);

    //Require call interval from 200 to 800 ms
    static bool update(void);

    static void set_enabled(bool enabled);
    static bool is_enabled() {
        return _enabled;
    }
};

#endif // DSP_MONITOR_PMS7003_H_
