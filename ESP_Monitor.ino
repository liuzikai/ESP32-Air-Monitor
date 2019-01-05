#include "Config.h"
#include "PMS7003.h"

#include <WiFi.h>
#include <HTTPClient.h>

#include "ThingSpeak.h"

#include <pt.h>



WiFiClient wifiClient;
HTTPClient httpClient;

void connect_and_login() {

    Serial.println("Attempting to connect to WiFi");

    while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(WIFI_SSID, WIFI_PASS);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
        Serial.print(".");
        delay(5000);     
    } 

    Serial.println("WiFi Connected");

    Serial.println("Attempting to login to ZJUWLAN");

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
        Serial.println("ZJUWLAN login succeeded.");
    } else {
        Serial.printf("ZJUWLAN login failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
    }

    httpClient.end();
}

static struct pt ptUpdatePMS7003;
static struct pt ptUploadData;

/**
 * The pt to retrieve data from PMS7003 sensor.
 * PMS7003 is control individually to receive data in time and avoid checksum error
 */
static int thdUpdatePMS7003(struct pt *pt) {
    PT_BEGIN(pt);
    while(1) {
        // Wait until the information is fully received
        PT_WAIT_UNTIL(pt, PMS7003::data_available());
        PMS7003::update();
    }
    PT_END(pt);
}

/**
 * The pt to upload data to ThingSpeak
 */
static int thdUploadData(struct pt *pt) {
    static unsigned long last_update_time = 0;
    PT_BEGIN(pt);
    while(1) {

        // Wait for 20s
        PT_WAIT_UNTIL(pt, millis() > last_update_time + 20000);
        last_update_time = millis();
        
        if(WiFi.status() != WL_CONNECTED){
            connect_and_login();
        }

        ThingSpeak.setField(1, PMS7003::pm1_0_amb);
        ThingSpeak.setField(2, PMS7003::pm2_5_amb);
        ThingSpeak.setField(3, PMS7003::pm10_0_amb);
        int x = ThingSpeak.writeFields(THINGSPEAK_CH_ID, THINGSPEAK_WRITE_APIKEY);
        if(x == 200){
            Serial.println("Channel update successful.");
        }
        else{
            Serial.println("Problem updating channel. HTTP error code " + String(x));
        }
        
    }
    PT_END(pt);
}


void setup() {

    // Start the serial
    Serial.begin(115200);

    // Attempt to connect to WiFi
    WiFi.mode(WIFI_STA);
    if(WiFi.status() != WL_CONNECTED){
        connect_and_login();
    }

    // Set ThingSpeak
    ThingSpeak.begin(wifiClient);

    // Initialize sensors
    PMS7003::begin();

    // Initialize threads
    PT_INIT(&ptUpdatePMS7003);
    PT_INIT(&ptUploadData);
}

void loop()
{
    thdUpdatePMS7003(&ptUpdatePMS7003);
    thdUploadData(&ptUploadData);
}
