void ldr_init();
int  ldr_get();
int  ldr_addavg(int n);
int  ldr_getavg();
int  ldr_setavg(int n);


#define LDR_PIN A0
#define LDR_MINCHANGE 10
#define LDR_AVGTIME 500


int _ldravg[60]; 
int _ldravglen = sizeof(_ldravg) / sizeof(int);
unsigned long _ldrtime;
int _ldravgpos=0;

void ldr_init()
{
      _ldrtime = 0;
      ldr_setavg(0);

      Serial.print("LDR ");
      Serial.println(ldr_get());
      
}

int  ldr_addavg(int n)
{
    _ldravg[_ldravgpos]=n;
    _ldravgpos++;
    if (_ldravgpos>=_ldravglen)
        _ldravgpos=0;
}

int  ldr_getavg()
{
   unsigned long s=0;
   for(int i=0;i<_ldravglen;i++)
      s += _ldravg[i];
   return  s/_ldravglen;  
}

int  ldr_setavg(int n)
{   for(int i=0;i<_ldravglen;i++)
      _ldravg[i] = n; 
}

int  ldr_get()
{
    int ldr = analogRead(LDR_PIN);

    if (abs(ldr-ldr_getavg())>LDR_MINCHANGE)
    {   ldr_setavg(ldr);
    } else
    {   if ( (millis()-_ldrtime)>LDR_AVGTIME )
        {   ldr_addavg(ldr);
            _ldrtime = millis();
        }
    }        
    return ldr_getavg();
}
