


#define VERSION "Version 1.0"
#define DATE "AUG 2021"

#include "bottle_clock_screen.h"
#include "ldr.h"
#include <Wire.h>

#define I2CADDRESS 4

#define TRANSITIONTIME 3000

#define CONTROLTEST 99
#define TESTLIGHT 100 

#define TESTPATTERN 8888

#define NOTIME (-1)

#define DISABLE_TIME_MIN 60 


int _light = TESTLIGHT;
int _time = NOTIME;
unsigned long _timestamp=0;
int _timestamp_h = NOTIME;
int _timestamp_m = NOTIME;

byte _ctrl[3];
bool _gotcontrol=false;



char _ldrdata[3] = {0};

void setup()
{
  Serial.begin(115200);

  Wire.begin(I2CADDRESS);
  Wire.onReceive(I2C_receive_event);
  Wire.onRequest(I2C_request_event); 

  
  bc_init();

  delay(300);

  Serial.print("\r\n* Bottle Clock ATMEL * ");
  Serial.print(VERSION);
  Serial.print(" ");
  Serial.println(DATE);


  Serial.print("Testing... ");
  bc_show(TESTPATTERN,TESTLIGHT);
  delay(3000);

  Serial.println("DONE");

  ldr_init();


  bc_show_transition(TESTPATTERN,_time,TESTLIGHT,5000);
 
  
}



void process_control(int light)
{
    if (!_gotcontrol)
      return;
    _gotcontrol=false;
 
    int h = _ctrl[0]; 
    int m = _ctrl[1]; 
    int s = _ctrl[2]; 
    
    if (h==CONTROLTEST)
    {
        Serial.print("Test #");
        Serial.print(m);
        Serial.println("");
        bc_test(m,light);
        _timestamp_h = NOTIME;
        _timestamp_m = NOTIME;             
        return;
    }
    
    Serial.print("Got time : ");
    Serial.print(h);
    Serial.print(":");
    Serial.print(m);
    Serial.print(":");
    Serial.print(s);
    Serial.println("");
    
    if ((h>=24) || (m>=60) || (s>=60) || (h<0) || (m<0) || (s<0))
    {    Serial.println("Wrong time. Skiped."); 
         return;
    }
    
    _timestamp_h = h;
    _timestamp_m = m;  
    _timestamp = millis();
    unsigned long dt = ((unsigned long)s)*1000;
    if (_timestamp>dt)
      _timestamp-=dt;

        
}





void loop()
{


    int ldr = ldr_get();
    int light = ldr_to_light(ldr);

    process_control(light);

    
    int t = NOTIME;

    _ldrdata[0] = ldr & 0xFF;
    _ldrdata[1] = (ldr & 0xFF00) >> 8;
  
  
    if (_timestamp_h != NOTIME)
    {
        unsigned long dt_min = ( (millis() -  _timestamp) / 1000) / 60;
        if(dt_min>DISABLE_TIME_MIN)
        {
           Serial.println("Clock disabled"); 
           t = NOTIME;     
           _timestamp_h = NOTIME;
           _timestamp_m = NOTIME;             
        } else
        {
            int m = _timestamp_m+dt_min;
            int dh = m / 60;
            m = m % 60;
            int h = (_timestamp_h+dh) % 24;
            t = h*100+m;
        }
     }
      
      if (t!=_time)
      {
        Serial.print("Clock ");
        Serial.print(_time);
        Serial.print(" -> ");
        Serial.println(t);
      } 

      if (_light>light)
      {
        _light--;
        //Serial.print("Light (-) ");
        //Serial.println(_light);
      }

      if (_light<light)
      {
        _light++;
        //Serial.print("Light (+) ");
        //Serial.println(_light);
      }

      
      bc_show_transition(_time,t,_light,TRANSITIONTIME);

      _time = t;

      delay(50);
  
  
}

int ldr_to_light(int ldr)
{

  int light = ldr/2;

  if (light==0)
    light =1;
  return light;

}






// three bytes
// 1) set time:  hours  minutes seconds 
// 2) run test: 99   <test index>  0 
void I2C_receive_event(int howMany)
{

  for(int i=0;i<3;i++)
    _ctrl[i] = Wire.read(); 

  while(Wire.available()) // skip data
    char c = Wire.read(); 

  _gotcontrol = true;

}


// two bytes LDR value
void I2C_request_event()
{
  Wire.write(_ldrdata);
}
