
#include <Arduino.h>
#include <util/atomic.h>

#define NUM_AXES  9        // 9 axes, X, Y, Z, etc

#define PULSE_MIN   (750)       // minimum PWM pulse width which is considered valid
#define PULSE_MAX   (2250)      // maximum PWM pulse width which is considered valid

static volatile uint16_t data[NUM_AXES];
static volatile uint16_t channels[NUM_AXES];


typedef enum {
  NULL_STATE = -1, NOT_SYNCHED_STATE, ANALIZE_STATE, READY_STATE, ERROR_STATE
} decodeState_t;

#define ANALIZE_COUNT  8  // must have this many consecutive valid frames to transition to the ready state.
static volatile byte channel;      // hold current cahnnel (first channel is 0)
static volatile byte detected_channels; // the total number of channels detected in a complete frame
static volatile decodeState_t State;         // hold current state
static volatile byte stateCount;         // counts the number of times this state has been repeated


static void checkState() {

  if (State == READY_STATE) {
    if (channel != detected_channels) { // if the number of channels is unstable, mark frame as error
      State = ERROR_STATE;
    }
  } else if (State == NOT_SYNCHED_STATE) {
    State = ANALIZE_STATE;        // this is the first sync pulse, we need one more to fill the channel data array
    stateCount = 0;
  }  else if ( State == ANALIZE_STATE) {
    if (++stateCount > ANALIZE_COUNT) {

      State = READY_STATE;           // this is the second sync and all channel data is ok so flag that channel data is valid
      detected_channels = channel; // save the number of channels detected
    }

  }  else if ( State == ERROR_STATE) {
    if (channel == detected_channels) { // did we get good data on all channels
      State = READY_STATE;
    }
  }

  channel = 0;       // reset the channel counter
}

static void interruptHandler()
{
  uint16_t diff;
  static uint16_t now;
  static uint16_t last = 0;

  last = now;
  now = micros();
  diff = now - last; //calculate time for pulse

  if ( diff > 2700) { // is the space between data big enough to be the SYNC
    checkState();
  } else if (channel < NUM_AXES) {
    if ( (diff > PULSE_MIN && diff < PULSE_MAX ) ) { // check for valid channel data
      data[channel++] = diff;  // store pulse length as microseconds, TODO: try to average value (old value + new value / 2), maybe i need to use long types!
    } else if (State == READY_STATE) {
      State = ERROR_STATE;  // set error flag
      channel = 0; // reset the channel count
    }
  }

}



void loop()
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    if ( (State == READY_STATE)) {
      memcpy((void*)channels, (void*)data, sizeof(data));
    }
  }
  Serial.write((uint8_t *)&channels, sizeof(channels));
  delay(100);
}

void setup()
{
  memset((void*)channels, 0, sizeof(channels));
  memset((void*)data, 0, sizeof(data));

  // Set up to listen on interrupt 1 which is digital input pin D3
  // See http://arduino.cc/en/Reference/attachInterrupt for mapping
  // interrupt number to pin number
  attachInterrupt(1, interruptHandler, RISING);
  Serial.begin(115200);
  delay(200);
}

