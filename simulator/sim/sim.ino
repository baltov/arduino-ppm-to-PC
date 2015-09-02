
//#define USE_RC

#include <Arduino.h>
#include <util/atomic.h>

#define NUM_AXES  4        // 9 axes, X, Y, Z, etc

#ifndef USE_RC
#define PULSE_MIN   (750)       // minimum PWM pulse width which is considered valid
#define PULSE_MAX   (2250)      // maximum PWM pulse width which is considered valid

//#define DEBUG

static volatile uint16_t data[NUM_AXES];
static volatile uint16_t channels[NUM_AXES];
    
    
typedef enum {
   NULL_state=-1, NOT_SYNCHED_state, ACQUIRING_state, READY_state, FAILSAFE_state    
} decodeState_t;

#define ACQUISITION_COUNT  8  // must have this many consecutive valid frames to transition to the ready state.
static volatile byte Channel;      // number of channels detected so far in the frame (first channel is 1)
static volatile byte NbrChannels; // the total number of channels detected in a complete frame
static volatile decodeState_t State;         // this will be one of the following states:
static volatile byte stateCount;         // counts the number of times this state has been repeated


static void processSync(){
// Sync was detected so reset the channel to 1 and update the system state       
  if(State == READY_state) {   
        if( Channel != NbrChannels){  // if the number of channels is unstable, go into failsafe
              State = FAILSAFE_state;
        }
  }
  else{ 
    if(State == NOT_SYNCHED_state){
            State = ACQUIRING_state;        // this is the first sync pulse, we need one more to fill the channel data array    
            stateCount = 0;
      }
      else if( State == ACQUIRING_state)      {
         if(++stateCount > ACQUISITION_COUNT) {
           State = READY_state;           // this is the second sync and all channel data is ok so flag that channel data is valid     
               NbrChannels = Channel; // save the number of channels detected
           }
    }
      else if( State == FAILSAFE_state)      {  
            if(Channel == NbrChannels){  // did we get good data on all channels
                 State = READY_state;
           }
      }
  }
  Channel = 0;       // reset the channel counter 
}

static void interruptHandler()
{
  uint16_t diff;
  static uint16_t now;
  static uint16_t last = 0;
  #ifdef DEBUG
  static uint16_t min = 64000;
  static uint16_t max = 0;
  #endif
  

  last = now;
  now = micros();
  diff = now - last;
  
  if( diff > 2700){   // is the space between data big enough to be the SYNC
     processSync();
   } else if(Channel < NUM_AXES) {  // check if its a valid channel pulse and save it
     if( (diff > PULSE_MIN && diff < PULSE_MAX ) ){ // check for valid channel data                  
          data[++Channel] = diff;  // store pulse length as microsoeconds
     }
     else if(State == READY_state){
       State = FAILSAFE_state;  // use fail safe values if input data invalid  
       Channel = 0; // reset the channel count
     }
   }   
  
//
//  if (diff > 3000 /*2700*/)  { // Per http://www.rcgroups.com/forums/showpost.php?p=21996147&postcount=3960 "So, if you use 2.5ms or higher as being the reset for the PPM stream start, you will be fine. I use 2.7ms just to be safe."
//    chan = 0;
//  }
//  else {
//    if (diff > PULSE_MIN && diff < PULSE_MAX && chan < 4 /*NUM_AXES*/) {   // 750 to 2250 ms is our 'valid' channel range
//     #ifdef DEBUG
//    
//     char buffer[7];         //the ASCII of the integer will be stored in this char array
//     itoa(chan,buffer,10);
//     
//     char buffer2[7];
//     itoa(diff,buffer2,10);
//
//     char minbuff[7];
//     itoa(min,minbuff,10);
//     char maxbuff[7];
//     itoa(max,maxbuff,10);
//
//     
//     if (diff > max) max = diff;
//     if (diff < min) min = diff;
//     Serial.print(buffer); Serial.print(":"); Serial.print(buffer2);Serial.print(":");
//     Serial.print(minbuff);Serial.print(":");Serial.println(maxbuff);
//     #endif
//
//     
//     diff = map(diff, 1068, 1916, 0, 1500);
//     diff= constrain(diff, 0, 1500); 
//     data[chan] = diff;
//    }
//    chan++;
//  }
  
}
#endif



void loop()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      if( (State == READY_state)){      
        memcpy((void*)channels, (void*)data, sizeof(data));
      }
    }
 #ifndef DEBUG
    Serial.write((uint8_t *)&channels, sizeof(channels));
  #endif
  delay(100);
}

void setup()
{
  memset((void*)channels,0,sizeof(channels));
  memset((void*)data,0,sizeof(data));  
  
  // Set up to listen on interrupt 1 which is digital input pin D3
  // See http://arduino.cc/en/Reference/attachInterrupt for mapping
  // interrupt number to pin number
  attachInterrupt(1, interruptHandler, RISING);
  Serial.begin(115200);
  delay(200);
}
/*
/*
* ServoDecode.cpp
* this version for Mega only
*/

//#define icpPin            8         // this interrupt handler must use pin 8
//#define TICKS_PER_uS      2          // number of timer ticks per microsecond
//
//
//typedef enum {
//   NULL_state=-1, NOT_SYNCHED_state, ACQUIRING_state, READY_state, FAILSAFE_state    
//} decodeState_t;
//
//#define NUM_AXES    8         // maximum number of channels we can store, don't increase this above 8
//#define MIN_IN_PULSE_WIDTH (750 * TICKS_PER_uS) //a valid pulse must be at least 750us (note clock counts in 0.5 us ticks)
//#define MAX_IN_PULSE_WIDTH (2250 * TICKS_PER_uS) //a valid pulse must be less than  2250us 
//#define SYNC_GAP_LEN      (3000 * TICKS_PER_uS) // we assume a space at least 3000us is sync (note clock counts in 0.5 us ticks)
//#define FAILSAFE_PIN   13  // if defined, this will set the given pin high when invalid data is received
//
//
//#define PULSE_START_ON_RISING_EDGE  0
//#define PULSE_START_ON_FALLING_EDGE (1<<ICES4) 
//#define ACQUISITION_COUNT  8  // must have this many consecutive valid frames to transition to the ready state.
//volatile byte pulseEnd = PULSE_START_ON_RISING_EDGE ; // default value
//static volatile unsigned int data[ NUM_AXES + 1]; // array holding channel data width value in microseconds 
//static volatile unsigned int Failsafe[NUM_AXES + 1]; // array holding channel fail safe values 
//static volatile byte Channel;      // number of channels detected so far in the frame (first channel is 1)
//static volatile byte NbrChannels; // the total number of channels detected in a complete frame
//static volatile decodeState_t State;         // this will be one of the following states:
//static volatile byte stateCount;         // counts the number of times this state has been repeated
//
//static void processSync(){
//// Sync was detected so reset the channel to 1 and update the system state       
//  data[0] = ICR4 / TICKS_PER_uS;  // save the sync pulse duration for debugging
//  if(State == READY_state) {   
//        if( Channel != NbrChannels){  // if the number of channels is unstable, go into failsafe
//              State = FAILSAFE_state;
//        }
//  }
//  else{ 
//    if(State == NOT_SYNCHED_state){
//            State = ACQUIRING_state;        // this is the first sync pulse, we need one more to fill the channel data array    
//            stateCount = 0;
//      }
//      else if( State == ACQUIRING_state)      {
//         if(++stateCount > ACQUISITION_COUNT) {
//           State = READY_state;           // this is the second sync and all channel data is ok so flag that channel data is valid     
//               NbrChannels = Channel; // save the number of channels detected
//           }
//    }
//      else if( State == FAILSAFE_state)      {  
//            if(Channel == NbrChannels){  // did we get good data on all channels
//                 State = READY_state;
//           }
//      }
//  }
//  Channel = 0;       // reset the channel counter 
//}
//
//ISR(TIMER4_OVF_vect){
// if(State == READY_state){
//   State = FAILSAFE_state;  // use fail safe values if signal lost  
//   Channel = 0; // reset the channel count
// }
//}
//
//ISR(TIMER4_CAPT_vect)
//{
// // we want to measure the time to the end of the pulse 
// if( (_SFR_BYTE(TCCR4B) & (1<<ICES4)) == pulseEnd ){         
//   TCNT1 = 0;       // reset the counter
//   if(ICR4 >= SYNC_GAP_LEN){   // is the space between data big enough to be the SYNC
//     processSync();
//   }    
//   else if(Channel < NUM_AXES) {  // check if its a valid channel pulse and save it
//     if( (ICR4 >= MIN_IN_PULSE_WIDTH)  && (ICR4 <= MAX_IN_PULSE_WIDTH) ){ // check for valid channel data                  
//       data[++Channel] = ICR4 / TICKS_PER_uS;  // store pulse length as microsoeconds
//     }
//     else if(State == READY_state){
//       State = FAILSAFE_state;  // use fail safe values if input data invalid  
//       Channel = 0; // reset the channel count
//     }
//   }      
// }
//} 
//
//ServoDecodeClass::ServoDecodeClass(){
//}
//
//void ServoDecodeClass::begin(){
// pinMode(icpPin,INPUT);
// Channel = 0;             
// State = NOT_SYNCHED_state;
// TCCR4A = 0x00;         // COM1A1=0, COM1A0=0 => Disconnect Pin OC from Timer/Counter  -- PWM11=0,PWM10=0 => PWM Operation disabled 
// TCCR4B = 0x02;         // 16MHz clock with prescaler means TCNT increments every .5 uS (cs11 bit set
// TIMSK4 = _BV(ICIE4)|_BV (TOIE4);   // enable input capture and overflow interrupts for timer 1
// for(byte chan = 1; chan <=  NUM_AXES; chan++) 
//       Failsafe[chan] = data[chan]= 1500; // set midpoint as default values for pulse and failsafe
//}
//
//decodeState_t ServoDecodeClass::getState(){
// return State;
//}
//
//byte ServoDecodeClass::getChanCount(){
// return NbrChannels;
//}
//
//void  ServoDecodeClass::setFailsafe(byte chan, int value){ 
//// pulse width to use if invalid data, value of 0 uses last valid data
// if( (chan > 0) && (chan <=  NUM_AXES)  ) {
//      Failsafe[chan] = value;
// }
//}
//void  ServoDecodeClass::setFailsafe(){ 
//// setFailsafe with no arguments sets failsafe for all channels to their current values
//// usefull to capture current tx settings as failsafe values 
// if(State == READY_state) 
//   for(byte chan = 1; chan <=  NUM_AXES; chan++) {
//       Failsafe[chan] = data[chan];
//   }
//}
//
//int ServoDecodeClass::GetChannelPulseWidth( uint8_t channel)
//{
// // this is the access function for channel data
// int result = 0; // default value
// if( channel <=  NUM_AXES)  {
//    if( (State == FAILSAFE_state)&& (Failsafe[channel] > 0 ) )
//        result = Failsafe[channel]; // return the channels failsafe value if set and State is Failsafe
//      else if( (State == READY_state) || (State == FAILSAFE_state) ){
//        cli();       //disable interrupts
//        result =  data[channel] ;  // return the last valid pulse width for this channel
//        sei(); // enable interrupts
//      }
// }
// return result;
//}
//// make one instance for the user 
//ServoDecodeClass ServoDecode = ServoDecodeClass() ;

