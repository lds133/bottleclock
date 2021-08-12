
//I2C MASTER

#include <stdint.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "my_esp.h"
#include "leds.h"
#include <Wire.h>
#define I2CADDRESS 4

#define GROUPID "bottleclock"

#define MQTTTOPIC_CLOCK "/home/clock"
#define MQTTTOPIC_CTRL  "/home/" GROUPID  "/ctrl" 
#define MQTTTOPIC_LIGHT "/home/" GROUPID  "/light" 
#define MQTTTOPIC_ONDUTY  "/home/on_duty"
#define CLIENT_ID  "ESP_" GROUPID 

#define CLIENT_LOGIN "mqtt_login"
#define CLIENT_PASSWORD "mqtt_password"

#define WIFISSID "my_wifi_ssif"
#define WIFIPASSWORD "my_wifi_password"
#define MQTTSERVERIP "192.168.0.1"


char ssid[] =  WIFISSID;
char password[] = WIFIPASSWORD;
char mqtt_server[] = MQTTSERVERIP;
char mqtt_user[] = CLIENT_LOGIN;
char mqtt_pass[] = CLIENT_PASSWORD;
char mqtt_client[] = CLIENT_ID;



#define PIN_YELLOWLED         13
#define PIN_BLUETOPLED        BUILTIN_LED   
#define LED_BLUE      0
#define LED_YELLOW    1
int  _ledpins[] =  {PIN_BLUETOPLED,PIN_YELLOWLED};
int  _ledhi[]   =  {LOW,HIGH};
int  _ledlo[]   =  {HIGH,LOW};
int _ledmode[] =  {LED_OFF,LED_OFF};


WiFiClient _wificlient;
PubSubClient _mqttclient(_wificlient);
MyESP _esp;
Leds _leds;

unsigned long _publishtimer = 0;
const unsigned long PUBLISHPERIOD = 30000;


#define MAX_CMD_LEN 20


// set time     '12:00:00' -> '/home/clock' 
// run test #2  '99:02:00' -> '/home/bottleclock/ctrl' 

void mqttcallback(char* topic, byte* payload, unsigned int len) 
{
    _leds.Set(LED_YELLOW,LED_FLASH);
    
    Serial.print("Message '");
    for (int i = 0; i < len; i++) 
    {   Serial.print((char)payload[i]);
    }
    Serial.println("'");

    if (len==0) 
        return;


    char text[MAX_CMD_LEN+1] = {0};
    int n = (len-1)>MAX_CMD_LEN ? MAX_CMD_LEN : (len-1);
    memcpy(text,payload,n);
    text[2] =0;
    text[5] =0;
    int h = atoi(text);
    int m = atoi(text+3);
    int s = atoi(text+6);
    Serial.print("SEND to slave : ");  
    Serial.print (h);
    Serial.print (":");
    Serial.print (m);
    Serial.print (":");
    Serial.print (s);
    Serial.println ("");
    Wire.beginTransmission(I2CADDRESS);
    Wire.write(h);
    Wire.write(m);
    Wire.write(s);
    Wire.endTransmission();   

}

void event(int eventid)
{   event_ex(eventid,0);
}


void event_ex(int eventid,int param)
{
    
    //Serial.print("Event: ");
    //Serial.print(eventid);
    //Serial.print(",");
    //Serial.print(param);
    //Serial.println("");
    
    switch(eventid)
    {   
    
        case EVENT_MQTTCONNECTING:
            Serial.println("MQTT connecting...");
            _leds.Set(LED_YELLOW,LED_BLINKLO);
            break;
            
        case EVENT_MQTTCONNECTED:
            Serial.println("MQTT connected");
            _leds.Set(LED_YELLOW,LED_OFF);
            _leds.Set(LED_BLUE,LED_OFF);
            _esp.Publish(MQTTTOPIC_ONDUTY,mqtt_client);
            _esp.Subscribe(MQTTTOPIC_CLOCK);
            _esp.Subscribe(MQTTTOPIC_CTRL);
            break;
            
        case EVENT_WIFICONNECTING:
            Serial.println("WIFI connecting...");
            _leds.Set(LED_BLUE,LED_BLINKLO); 
            break;
            
        case EVENT_WIFICONNECTED:
            Serial.println("WIFI connected");
            _leds.Set(LED_BLUE,LED_OFF); 
            break;
            
        case EVENT_ESPSTUCK:
            _leds.Set(LED_BLUE,LED_BLINKHI);
            _leds.Set(LED_YELLOW,LED_BLINKHI);        
            Serial.println("ESP Soft Reset in 10 sec...");
            safedelay(10000);            
            _esp.SoftReset();
            break;
            
        case EVENT_MQTTCONNECT_ERROR:
            Serial.println("MQTT connect error...");
            break;

  

    }
    
}

void setup()
{
    Serial.begin(115200);   
    delay(1000);
    Serial.print("\r\n\r\n* Bottle Clock ESP *\r\n");
    
    _leds.Setup( sizeof(_ledpins)/sizeof(int), _ledpins, _ledhi, _ledlo, _ledmode);

    Wire.begin(); 
    
    _esp.Setup(ssid,password,mqtt_server,mqtt_user,mqtt_pass,mqtt_client,&safedelay,&mqttcallback,&_wificlient,&_mqttclient,&event);        
    
    Serial.print("Light topic: ");
    Serial.println(MQTTTOPIC_LIGHT);
    Serial.print("Control topic: ");
    Serial.println(MQTTTOPIC_CTRL);
    Serial.print("Clock topic: ");
    Serial.println(MQTTTOPIC_CLOCK);
    Serial.print("\r\n\r\n");

}


void loop()
{

    unsigned long t = millis();

    _esp.Loop(&t);
    _leds.Loop(&t);

    if (t-_publishtimer>PUBLISHPERIOD)
    {
        _publishtimer = t;
            
        Wire.requestFrom(I2CADDRESS, 2); 
        char vl = Wire.read();
        char vh = Wire.read();
        int v = (vh << 8 ) | vl;
        while (Wire.available()) 
        {   char c = Wire.read(); 
        }
        Serial.print("RECV from slave : ");  
        Serial.print(v); 
        Serial.print(""); 

        if (v<1024)
        {   char text[10]={0};
            itoa(v, text, 10);
            _esp.Publish(MQTTTOPIC_LIGHT,text);
            Serial.println(" published"); 
        } else
        {
            Serial.println(" skipped");     
        }
    }

    delay(1);



    

}


void safedelay(unsigned long n_ms)
{
    unsigned long t_stop = millis()+n_ms;
    unsigned long t;
    while(true)
    {
        t = millis();
        if (t>t_stop)
            break;
        _leds.Loop(&t);
        delay(1);
    }
}
