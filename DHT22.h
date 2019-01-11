/**
 * A warpper class of Adafruit DHT lib to control DHT22 sensor.
 * Author: liuzikai
 * Date: June 2018
 */

#ifndef ENVIRONMENT_MONITOR_DHT22_ADAFRUIT_H_
#define ENVIRONMENT_MONITOR_DHT22_ADAFRUIT_H_

#include "DHTesp.h"

class DHT22_Sensor {
    
private:

    static constexpr int data_pin = 25;

    static DHTesp dht22;

public:

    static float temperature;
    static float humidity;

    static bool update();

    static void start() {
        dht22.setup(data_pin, DHTesp::DHT22);
    }

    static String get_error_status_str() {
        return dht22.getStatusString();
    }

};

#endif // ENVIRONMENT_MONITOR_DHT22_ADAFRUIT_H_
