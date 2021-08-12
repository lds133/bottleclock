
/*
    Basic Pin setup:
    ------------                                  ---u----
    ARDUINO   13|-> SCLK (pin 25)           OUT1 |1     28| OUT channel 0
              12|                           OUT2 |2     27|-> GND (VPRG)
              11|-> SIN (pin 26)            OUT3 |3     26|-> SIN (pin 11)
              10|-> BLANK (pin 23)          OUT4 |4     25|-> SCLK (pin 13)
               9|-> XLAT (pin 24)             .  |5     24|-> XLAT (pin 9)
               8|                             .  |6     23|-> BLANK (pin 10)
               7|                             .  |7     22|-> GND
               6|                             .  |8     21|-> VCC (+5V)
               5|                             .  |9     20|-> 2K Resistor -> GND
               4|                             .  |10    19|-> +5V (DCPRG)
               3|-> GSCLK (pin 18)            .  |11    18|-> GSCLK (pin 3)
               2|                             .  |12    17|-> SOUT
               1|                             .  |13    16|-> XERR
               0|                           OUT14|14    15| OUT channel 15
    ------------                                  --------

    -  Put the longer leg (anode) of the LEDs in the +5V and the shorter leg
         (cathode) in OUT(0-15).
    -  +5V from Arduino -> TLC pin 21 and 19     (VCC and DCPRG)
    -  GND from Arduino -> TLC pin 22 and 27     (GND and VPRG)
    -  digital 3        -> TLC pin 18            (GSCLK)
    -  digital 9        -> TLC pin 24            (XLAT)
    -  digital 10       -> TLC pin 23            (BLANK)
    -  digital 11       -> TLC pin 26            (SIN)
    -  digital 13       -> TLC pin 25            (SCLK)
    -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each
       LED.  To be precise, it's I = 39.06 / R (in ohms).  This doesn't depend
       on the LED driving voltage.
    - (Optional): put a pull-up resistor (~10k) between +5V and BLANK so that <-------------------------
                  all the LEDs will turn off when the Arduino is reset.

    If you are daisy-chaining more than one TLC, connect the SOUT of the first
    TLC to the SIN of the next.  All the other pins should just be connected
    together:
        BLANK on Arduino -> BLANK of TLC1 -> BLANK of TLC2 -> ...
        XLAT on Arduino  -> XLAT of TLC1  -> XLAT of TLC2  -> ...
    The one exception is that each TLC needs it's own resistor between pin 20
    and GND.

    This library uses the PWM output ability of digital pins 3, 9, 10, and 11.
    Do not use analogWrite(...) on these pins.

    This sketch does the Knight Rider strobe across a line of LEDs.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

#include "Tlc5940.h"


#define TEST_SEGMENTS    1
#define TEST_TRANSITION  2

const int BCCHANMAX = 32;

int BCDIGITS_0[10] = { 0b1111011 ,0b1100000, 0b1011101, 0b1110101, 0b1100110,0b0110111,0b0111111,0b1100001,0b1111111,0b1110111};
int BCDIGITS_1[10] = { 0b1101111 ,0b0000011, 0b1011101, 0b1010111, 0b0110011,0b1110110,0b1111110,0b1000011,0b1111111,0b1110111};


int BCDASH_0 =         0b0000100;
int BCDASH_1 =         0b0010000;


void bc_show_transition(int num0,int num1,int light,int delay_ms);
void bc_show(int num,int light);

void bc_init()
{

  Tlc.init();  

  Serial.println(NUM_TLCS);
  
}


void bc_set_digit_ex(int pattern,int* d,int pos,int light)
{
    for(int i = 0;i<7;i++)
       d[i+pos] = ((pattern & (1<<(6-i))) !=0) ? light : 0 ;
}


void bc_set_digit(int* pattern,int* d,int pos,int n,int light)
{
    bc_set_digit_ex(pattern[n],d,pos,light);
}







void bc_test_segments(int v)
{
  
    Tlc.clear();
    for(int j=0;j<=32;j++)
        Tlc.set(j, v );
    Tlc.update();
    delay(1000);


     Tlc.clear();
     Tlc.update();
     delay(1000);
  
}

void bc_test_transition(int v)
{
    const int DELAY = 3000;

    bc_show(0,v);
    delay(DELAY);

    for(int i = 1;i<1000;i++)
    {   bc_show_transition(i-1,i,v,DELAY);
        delay(DELAY);
    }
  
}



void bc_test(int index,int v)
{
  while(true)
  {
  
      if (index==TEST_SEGMENTS)
      {   bc_test_segments(v);
          continue;
      }

      if (index==TEST_TRANSITION)
      {   bc_test_transition(v);
          continue;
      }
      


      
  }  
}






void bc_set_num(int* d,int num,int light)
{

  if (num<0)
  {
    bc_set_digit_ex(BCDASH_1,d,8,light);
    bc_set_digit_ex(BCDASH_1,d,1,light);
    bc_set_digit_ex(BCDASH_0,d,17,light);
    bc_set_digit_ex(BCDASH_0,d,24,light);
    return;
  }

  
  const int MAXDIGITS = 4;
  int n[MAXDIGITS];
  for(int i=0;i<MAXDIGITS;i++)
  {  n[i] = num % 10;
     num /= 10;
  }


  bc_set_digit(BCDIGITS_1,d,8,n[0],light);
  bc_set_digit(BCDIGITS_1,d,1,n[1],light);

  bc_set_digit(BCDIGITS_0,d,17,n[2],light);
  bc_set_digit(BCDIGITS_0,d,24,n[3],light);


  
}


void bc_print(int* d)
{
  const int OFF = 0;
  const int LIGHTMAX = 1000;

  d[0] = OFF;
  d[15] = OFF;
  d[16] = OFF;
  d[31] = OFF;

  Tlc.clear();
  
  for(int i=0;i<=BCCHANMAX;i++)
  {
      int v  = (d[i]>LIGHTMAX) ? LIGHTMAX : d[i];
      Tlc.set(i, v);

  }
      
  Tlc.update();
}


void bc_show(int num,int light)
{
  int d[32] = {0};

  bc_set_num(d,num,light);

  bc_print(d);

  
}


void bc_show_transition(int num0,int num1,int light,int delay_ms)
{

  if (num0==num1)
  {
     bc_show(num0,light);
     return;
  }

  const int STEPDELAYMIN = 1;
  const int STEPDELAYMAX = 100;
  const int STEP = 1;
  int NSTEPS = light;
  int STEPDELAY = delay_ms/NSTEPS;

  if (STEPDELAY<STEPDELAYMIN)
    STEPDELAY = STEPDELAYMIN;
  if (STEPDELAY>STEPDELAYMAX)
    STEPDELAY = STEPDELAYMAX;
  
  int d0[32] = {0};
  int d1[32] = {0};
  
  bc_set_num(d0,num0,light);
  bc_set_num(d1,num1,light);


  bool isdone=false;

  while(!isdone)
  {
    bc_print(d0);
    isdone=true;
    for(int i=0;i<BCCHANMAX;i++)
    {
        if (d0[i] == d1[i])
          continue;
  
        isdone=false;    
        if (d0[i]>d1[i])
        {   d0[i] -= STEP;
            if (d0[i]<d1[i])  
              d0[i] =d1[i];
        } else
        {
            d0[i] += STEP;
            if (d0[i]>d1[i])  
              d0[i] = d1[i];        
          
        }
    }
    
    delay(STEPDELAY);
  }
}
