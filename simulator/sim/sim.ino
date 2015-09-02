
//#define USE_RC

#ifdef USE_RC
#include <RcTrainer.h>
#else
#include <Arduino.h>
#include <util/atomic.h>
#endif


#define NUM_AXES  4        // 9 axes, X, Y, Z, etc

#ifndef USE_RC
//#define PULSE_MIN   (750)       // minimum PWM pulse width which is considered valid
//#define PULSE_MAX   (2250)      // maximum PWM pulse width which is considered valid

#define PULSE_MIN   (760)       // minimum PWM pulse width which is considered valid
#define PULSE_MAX   (2280)      // maximum PWM pulse width which is considered valid

#else
 RcTrainer rc(1);
#endif

//#define DEBUG

uint16_t data[NUM_AXES] = {0, 0, 0, 0};//, 0, 0, 0, 0, 0};


#ifndef USE_RC
static void interruptHandler()
{
  uint16_t diff;
  static uint16_t now;
  static uint16_t last = 0;
  static uint8_t chan = 0;
  #ifdef DEBUG
  static uint16_t min = 64000;
  static uint16_t max = 0;
  #endif
  

  last = now;
  now = micros();
  diff = now - last;

  if (diff > 3000 /*2700*/)  { // Per http://www.rcgroups.com/forums/showpost.php?p=21996147&postcount=3960 "So, if you use 2.5ms or higher as being the reset for the PPM stream start, you will be fine. I use 2.7ms just to be safe."
    chan = 0;
  }
  else {
    if (/*diff > PULSE_MIN && diff < PULSE_MAX && */chan < 4 /*NUM_AXES*/) {   // 750 to 2250 ms is our 'valid' channel range
     #ifdef DEBUG
    
     char buffer[7];         //the ASCII of the integer will be stored in this char array
     itoa(chan,buffer,10);
     
     char buffer2[7];
     itoa(diff,buffer2,10);

     char minbuff[7];
     itoa(min,minbuff,10);
     char maxbuff[7];
     itoa(max,maxbuff,10);

     
     if (diff > max) max = diff;
     if (diff < min) min = diff;
     Serial.print(buffer); Serial.print(":"); Serial.print(buffer2);Serial.print(":");
     Serial.print(minbuff);Serial.print(":");Serial.println(maxbuff);
     #endif

     
     diff = map(diff, 1068, 1916, 0, 1500);
     diff= constrain(diff, 0, 1500); 
     data[chan] = diff;
    }
    chan++;
  }
}
#endif

uint16_t channels[NUM_AXES] = {0, 0, 0, 0};//, 0, 0, 0, 0, 0};

void loop()
{
  #ifndef USE_RC
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    memcpy(channels, data, sizeof(data));
  }
  
 #else

 uint8_t i;
 for (i=0;i<6;i++){
  uint16_t val = rc.getChannel(i);
  channels[i] = val;
 }
 #endif 
 #ifndef DEBUG
    Serial.write((uint8_t *)&channels, sizeof(channels));
  #endif

  delay(100);
}

void setup()
{
  // Set up to listen on interrupt 1 which is digital input pin D3
  // See http://arduino.cc/en/Reference/attachInterrupt for mapping
  // interrupt number to pin number

#ifndef USE_RC
  attachInterrupt(1, interruptHandler, RISING);
#endif
  Serial.begin(115200);
  delay(200);

}

