/**
 * A warpper class of Adafruit DHT lib to control DHT22 sensor.
 * Author: liuzikai
 * Date: June 2018
 */

#include "DHT22_Adafruit.h"

DHT DHT22_Sensor::dht22(DHT22_Sensor::data_pin, DHT22);

float DHT22_Sensor::temperature = 0.0;
float DHT22_Sensor::humidity = 0.0;

bool DHT22_Sensor::update() {
    float _temperature = dht22.readTemperature();
    float _humidity = dht22.readHumidity();
    if (!isnan(_temperature) && !isnan(_humidity)) {
        temperature = _temperature;
        humidity = _humidity;
        return true;
    }
    return false;
}