#include <Arduino.h>
#include <util/atomic.h>

#define NUM_AXES  9        // 9 axes, X, Y, Z, etc

#define PULSE_MIN   (750)       // minimum PWM pulse width which is considered valid
#define PULSE_MAX   (2250)      // maximum PWM pulse width which is considered valid

typedef struct joyReport_t {
  int16_t axis[NUM_AXES];
} joyReport_t;

vo joyReport_t joyReport = {{0,0,0,0,0,0,0,0,0}};

void setup()
{
  // Set up to listen on interrupt 1 which is digital input pin D3
  // See http://arduino.cc/en/Reference/attachInterrupt for mapping
  // interrupt number to pin number

  attachInterrupt(1, interruptHandler, RISING);
  Serial.begin(115200);
  delay(200);
}


static void interruptHandler()
{
  uint16_t diff;
  static uint16_t now;
  static uint16_t last = 0;
  static uint8_t chan = 0;

  last = now;
  now = micros();
  diff = now - last;

  if (diff > 2700) { // Per http://www.rcgroups.com/forums/showpost.php?p=21996147&postcount=3960 "So, if you use 2.5ms or higher as being the reset for the PPM stream start, you will be fine. I use 2.7ms just to be safe."
    chan = 0;
  } 
  else {
    if (diff > PULSE_MIN && diff < PULSE_MAX && chan < NUM_AXES) {   // 750 to 2250 ms is our 'valid' channel range
      joyReport.axis[chan] = diff;
    }
    chan++;
  }
}


void loop()
{
  uint16_t channels[NUM_AXES] = {0,0,0,0,0,0,0,0,0};
  uint8_t i;
  for (i=0; i< NUM_AXES;i++) 
  {
    uint16_t chan;
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
      chan = joyReport.axis[i];
    } // after this line interrupts are allways enabled!If you want state to be restored use ATOMIC_RESTORESTATE

    uint16_t val = map(chan,PULSE_MIN,PULSE_MAX,0,2000);
    val = constrain(val, 0,2000);
    channels[i] = val;
  }
  Serial.write((uint8_t *)&channels, sizeof(channels));
  delay(100);
}

