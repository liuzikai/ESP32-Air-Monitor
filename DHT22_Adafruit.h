/**
 * A warpper class of Adafruit DHT lib to control DHT22 sensor.
 * Author: liuzikai
 * Date: June 2018
 */

#ifndef ENVIRONMENT_MONITOR_DHT22_ADAFRUIT_H_
#define ENVIRONMENT_MONITOR_DHT22_ADAFRUIT_H_

#include <DHT.h>

class DHT22_Sensor {
    
private:

    static constexpr int data_pin = 25;

    static DHT dht22;

public:

    static float temperature;
    static float humidity;

    static bool update();

};

#endif // ENVIRONMENT_MONITOR_DHT22_ADAFRUIT_H_