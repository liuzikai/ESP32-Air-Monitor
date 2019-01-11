/**
 * A warpper class of Adafruit DHT lib to control DHT22 sensor.
 * Author: liuzikai
 * Date: June 2018
 */

#include "DHT22.h"

DHTesp DHT22_Sensor::dht22;

float DHT22_Sensor::temperature = 0.0;
float DHT22_Sensor::humidity = 0.0;

bool DHT22_Sensor::update() {
    // float _temperature = dht22.readTemperature();
    // float _humidity = dht22.readHumidity();
    TempAndHumidity newValues = dht22.getTempAndHumidity();
    if (dht22.getStatus() == 0) {
        temperature = newValues.temperature;
        humidity = newValues.humidity;
        return true;
    }
    return false;
}