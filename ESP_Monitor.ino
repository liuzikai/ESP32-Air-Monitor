/**
 * DEBUG_ENABLE_SERIAL
 * @breif enable feedback serial
 * @default 0
 * @note if the dev board is not connect to PC, this may cause block in serial print.
 */
#define DEBUG_ENABLE_SERIAL 0

#if DEBUG_ENABLE_SERIAL
#define SERIAL_PRINTLN(s) Serial.println(s)
#define SERIAL_PRINTF(...) Serial.printf(__VA_ARGS__);
#else
#define SERIAL_PRINTLN(s)
#define SERIAL_PRINTF(...)
#endif

/**
 * DEBUG_LOCAL_WORK
 * @breif disable WiFi connection functions and upload task
 * @default 0
 * @note only for debug. Make sure serial is enabled in the debug mode, or you will have no way to get message.
 *       And there is no sensor sleep control!
 */
#define DEBUG_LOCAL_WORK 0



#define WIFI_RETRY_INTERVAL 5000 // ms
#define UPLOAD_INTERVAL 60000 // ms
#define UPLOAD_RETRY_INTERVAL 2000 // ms
#define PMS7003_PRE_START_TIME 5000 //ms
#define DHT22_PRE_START_TIME 10000 //ms
#define DHT22_INTERVAL 2500 //ms
#define DHT22_RETRY_INTERVAL 1000 //ms


#include "Config.h"

#include "PMS7003.h"
#include "DHT22.h"

#if !DEBUG_LOCAL_WORK
#include <WiFi.h>
#include <HTTPClient.h>
#include "ThingSpeak.h"
#endif

#include <pt.h>

#if !DEBUG_LOCAL_WORK

WiFiClient wifiClient;
HTTPClient httpClient;

void connect_and_login() {

    SERIAL_PRINTLN("Attempting to connect to WiFi");

    while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(WIFI_SSID, WIFI_PASS);  // Connect to WPA/WPA2 network
        SERIAL_PRINTF(".");
        delay(WIFI_RETRY_INTERVAL);     
    } 

    SERIAL_PRINTLN("WiFi Connected");
    SERIAL_PRINTLN("Attempting to login to ZJUWLAN");

    httpClient.begin("http://10.105.1.35:802/include/auth_action.php");
    httpClient.addHeader ("Content-Type", "application/x-www-form-urlencoded");
    // httpClient.addHeader ("Origin", "http://10.105.1.35:802");
    // httpClient.addHeader ("Accept", "*/*");
    // httpClient.addHeader ("Accept-Language", "en-us");
    // httpClient.addHeader ("Accept-Encoding", "gzip, deflate");
    // httpClient.addHeader ("Cookie", "login=bQ0o5ZADI11BgO3HLndd%252Bxt3LbV4WDOuk6o4S7LPZbks%252BAhUaMMBXlgnRUlpF784SsByEEvF2CzJ5E5gaRQ7ss%252Fd3%252Fb6I8H8vgSNxsG79%252BfXsTGorWHwQAfp1g22kR%252BryyK4BwixNciWLyisJeV9K%252FnO4fcd0Vvk8z4Lbdhn230rvsUrwEU%253D; double_stack_login=bQ0pOyR6IX%252Fu0akbf5QES0glrbCtHF7Gw%252Bt41YkQ1se6N0TVgn6piexRmcH4cjZGbKq2%252FVaTFX29E1RuLaaP9k1mmDnOuFcdZ2XcHN0FyBwlQu5c%252FxNAsqP94QTf7sfaUDrmPcV1H96N7ftv5nXhNIWVhWiaNXOcLaqvVy3EB3ABq4HsTepfgYLZ3aOZWPsU5RAaYYv%252Fficg8AIXXr5qdw%253D%253D; login=bQ0o5ZADI11BgO3HLndd%252Bxt3LbV4WDOuk6o4S7LPZbks%252BAhUaMMBXlgnRUlpF784SsByEEvF2CzJ5E5gaRQ7ss%252Fd3%252Fb6I8H8vgSNxsG79%252BfXsTGorWHwQAfp1g22kR%252BryyK4BwixNciWLyisJeV9K%252FnO4fcd0Vvk8z4Lbdhn230rvsUrwEU%253D");
    httpClient.addHeader ("Referer", "http://10.105.1.35:802/srun_portal_pc_en.php?ac_id=12&&ac_id=12");
    int httpCode = httpClient.POST("action=login"
                                   "&username=" WIFI_ZJUWLAN_INTL_ID "%2540intl.zju.edu.cn"
                                   "&password=" WIFI_ZJUWLAN_PASS 
                                   "&ac_id=12&user_ip=&nas_ip=&user_mac=&save_me=0&ajax=1");

    if(httpCode == HTTP_CODE_OK) {
        String payload = httpClient.getString();
        SERIAL_PRINTLN("ZJUWLAN login succeeded.");
    } else {
        SERIAL_PRINTF("ZJUWLAN login failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
    }
    httpClient.end();
}

#endif

static struct pt ptPMS7003Interupt;
static struct pt ptUpdateDHT22;
static struct pt ptUploadData;

static unsigned long  dht22_next_sample_time = DHT22_INTERVAL;

/**
 * Thread to retrieve data from PMS7003 sensor.
 * PMS7003 is control individually to receive data in time and avoid checksum error. 
 * Whenever there is available message, the program will retrieve and process it.
 * There is no sample time control for PMS7003. Use PMS7003::set_enable() to control the sensor directly.
 */
static int thdPMS7003Interupt(struct pt *pt) {
    PT_BEGIN(pt);
    while(1) {
        // Wait until the information is fully received, which is the only condition.
        PT_WAIT_UNTIL(pt, PMS7003::data_available());
        PMS7003::update();
        SERIAL_PRINTF("    PMS7003: PM1.0: %u, PM2.5: %u, PM10: %u", PMS7003::pm1_0_amb, PMS7003::pm2_5_amb, PMS7003::pm10_0_amb);
        SERIAL_PRINTLN("");
    }
    PT_END(pt);
}

/**
 * Thread to update other DHT22.
 * Control by dht22_next_sample_time. This thread can set it for repeating sampling, 
 * and thdUploadData can also extend the interval by setting it to next_upload_time - DHT22_PRE_START_TIME
 */
static int thdUpdateDHT22(struct pt *pt) {
    PT_BEGIN(pt);
    while(1) {
        // Wait until next sample time is reached
        PT_WAIT_UNTIL(pt, (millis() > dht22_next_sample_time));

        if (DHT22_Sensor::update()) {

            SERIAL_PRINTF("    DHT22: temperature: %f, humidity: %f", DHT22_Sensor::temperature, DHT22_Sensor::humidity);
            SERIAL_PRINTLN("");

            dht22_next_sample_time = millis() + DHT22_INTERVAL;  // success, perform next sample after DHT22_INTERVAL
        } else {

            SERIAL_PRINTLN("    DHT22 sample failed");

            dht22_next_sample_time = millis() + DHT22_RETRY_INTERVAL;  // failed, retry after 1s
        }

    }
    PT_END(pt);
}

#if !DEBUG_LOCAL_WORK
/**
 * Thread to upload data to ThingSpeak
 */
static int thdUploadData(struct pt *pt) {
    static unsigned long next_upload_time = UPLOAD_INTERVAL;
    PT_BEGIN(pt);
    while(1) {

        if (!PMS7003::is_enabled()) {
            // If PMS7003 is not enabled, enable it ahead
            PT_WAIT_UNTIL(pt, millis() > next_upload_time - PMS7003_PRE_START_TIME);
            PMS7003::set_enabled(true);

            SERIAL_PRINTLN("PMS7003 enabled.");

            // Continue to wait for real upload time
        }

        // Wait for next update time
        PT_WAIT_UNTIL(pt, millis() > next_upload_time);

        if(WiFi.status() != WL_CONNECTED){
            // This can block the thread trying to connect to WiFi, so if WiFi is not connect, DHT22 won't blink.
            connect_and_login();
        }

        ThingSpeak.setField(1, PMS7003::pm1_0_amb);
        ThingSpeak.setField(2, PMS7003::pm2_5_amb);
        ThingSpeak.setField(3, PMS7003::pm10_0_amb);
        ThingSpeak.setField(4, DHT22_Sensor::temperature);
        ThingSpeak.setField(5, DHT22_Sensor::humidity);
        int x = ThingSpeak.writeFields(THINGSPEAK_CH_ID, THINGSPEAK_WRITE_APIKEY);

        if(x == 200){

            SERIAL_PRINTLN("Channel update successful.");

            next_upload_time += UPLOAD_INTERVAL;  // success, perform next upload after UPLOAD_INTERVAL

            dht22_next_sample_time = next_upload_time - DHT22_PRE_START_TIME;  // control DHT22 by setting next update sample time

            PMS7003::set_enabled(false);  // control PMS7003 by setting enabled


            SERIAL_PRINTLN("PMS7003 disabled.");
            SERIAL_PRINTLN("DHT22 next sample time extended.");

        }
        else{

            SERIAL_PRINTLN("Problem updating channel. HTTP error code " + String(x));

            next_upload_time += UPLOAD_RETRY_INTERVAL;  // failed, retry after UPLOAD_RETRY_INTERVAL
        }
    }
    PT_END(pt);
}

#endif

void setup() {

#if DEBUG_ENABLE_SERIAL
    // Start the serial
    Serial.begin(115200);
    // Serial.setTimeout(1000);
#endif

#if !DEBUG_LOCAL_WORK
    // Attempt to connect to WiFi
    WiFi.mode(WIFI_STA);
    if(WiFi.status() != WL_CONNECTED){
        connect_and_login();
    }

    // Set ThingSpeak
    ThingSpeak.begin(wifiClient);
#endif

    // Initialize sensors
    PMS7003::begin();
    DHT22_Sensor::start();

    // Initialize threads
    PT_INIT(&ptPMS7003Interupt);
    PT_INIT(&ptUpdateDHT22);
#if !DEBUG_LOCAL_WORK
    PT_INIT(&ptUploadData);
#endif
}

void loop()
{
    thdPMS7003Interupt(&ptPMS7003Interupt);
    thdUpdateDHT22(&ptUpdateDHT22);
#if !DEBUG_LOCAL_WORK
    thdUploadData(&ptUploadData);
#endif
}
