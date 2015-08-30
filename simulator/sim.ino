#include <RcTrainer.h>

// dx61.ino
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2013 Mike McCauley
//
// Print out servo positions from a Spektrum DX6i in trainer mode

#include <RcTrainer.h>

// Set up to listen on interrupt 0 which is digital input pin D2
// See http://arduino.cc/en/Reference/attachInterrupt for mapping
// interrupt number to pin number
RcTrainer tx(1);

//#define NUM_BUTTONS  40
#define NUM_AXES  9        // 9 axes, X, Y, Z, etc
typedef struct joyReport_t {
  int16_t axis[NUM_AXES];
  //uint8_t button[(NUM_BUTTONS+7)/8]; // 8 buttons per byte
} joyReport_t;

joyReport_t joyReport = {{0,0,0,0,0,0,0,0,0}};//, {0,0,0,0,0}};

void setup()
{
    Serial.begin(115200);
    //Serial.begin(19200);
    delay(200);
   // Serial.begin(14400);

}


void loop()
{
    //Serial.print("----------------------\n");
    // Default mapping is used, suitable for DX6i, which has 6 channels
    uint8_t i;
    for (i = 0; i < NUM_AXES; i++){
	    joyReport.axis[i] = (uint16_t) tx.getChannel(i);
    // Serial.println(joyReport.axis[i]);
    }
 
    Serial.write((uint8_t *)&joyReport, sizeof(joyReport));
    delay(100);
}
