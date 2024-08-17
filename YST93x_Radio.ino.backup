#include <inttypes.h>
#include <SanyoCCB.h>
#include <Arduino.h>
#include <TM1637Display.h>

// LC72131 IN1, byte 0
#define IN10_R3     7
#define IN10_R2     6
#define IN10_R1     5
#define IN10_R0     4
#define IN10_XS     3
#define IN10_CTE    2
#define IN10_DVS    1
#define IN10_SNS    0

// LC72131 IN2, byte 0
#define IN20_TEST2  7
#define IN20_TEST1  6
#define IN20_TEST0  5
#define IN20_IFS    4
#define IN20_DLC    3
#define IN20_TBC    2
#define IN20_GT1    1
#define IN20_GT0    0

// LC72131 IN2, byte 1
#define IN21_DZ1    7
#define IN21_DZ0    6
#define IN21_UL1    5
#define IN21_UL0    4
#define IN21_DOC2   3
#define IN21_DOC1   2
#define IN21_DOC0   1
#define IN21_DNC    0

// LC72131 IN2, byte 2
#define IN22_BO4    7
#define IN22_BO3    6
#define IN22_BO2    5
#define IN22_BO1    4
#define IN22_IO2    3
#define IN22_IO1    2
#define IN22_IOC2   1
#define IN22_IOC1   0

// LC72131 DO0, byte 0
#define DO0_IN2     7
#define DO0_IN1     6
#define DO0_UL      4

// For function YST93xSetMode
#define YST93x_MONO    1
#define YST93x_STEREO  2
#define YST93x_MUTE    3
#define YST93x_UNMUTE  4
#define YST93x_BAND_FM 5
#define YST93x_BAND_AM 6

#define FM_TUNED_WINDOW 180
#define AM_TUNED_WINDOW 20

const int CLK = 8; //Set the CLK pin connection to the display
const int DIO = 7; //Set the DIO pin connection to the display

//TM1637Display display(CLK, DIO); //set up the 4-Digit Display.
TM1637Display display(CLK, DIO); //set up the 4-Digit Display.
SanyoCCB ccb(3, 2, 4, 5); // Pins: DO CL DI CE

byte pll_in1[3];
byte pll_in2[3];

// Initial frequencies
uint16_t FMFrequency = 937;   // MHz * 10
uint16_t AMFrequency = 52;    // KHZ / 10

uint8_t band = YST93x_BAND_FM;
uint8_t tuned = 0;

//smoothing input
const int numReadings =40;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;

void YST93xInit() {
  memset(pll_in1, 0, 3);
  memset(pll_in2, 0, 3);
  bitSet(pll_in2[0], IN20_IFS);   // IF counter in normal mode
  bitSet(pll_in2[1], IN21_UL0);   // Phase error detection width = 0us
  bitSet(pll_in2[2], IN22_BO2);   // Mute off / normal tuner mode
  bitSet(pll_in2[2], IN22_BO4);
}

void YST93xSetMode(uint8_t mode) {
  switch (mode) {
    case YST93x_STEREO:
      bitClear(pll_in2[2], IN22_BO3);
      break;

    case YST93x_MONO:
      bitSet(pll_in2[2], IN22_BO3);
      break;

    case YST93x_MUTE:
      bitClear(pll_in2[2], IN22_BO2);
      break;

    case YST93x_UNMUTE:
      bitSet(pll_in2[2], IN22_BO2);
      break;

    case YST93x_BAND_FM:
      band = YST93x_BAND_FM;
      bitWrite(pll_in1[0], IN10_R0,  1); // Reference frequency = 50kHz
      bitWrite(pll_in1[0], IN10_R3,  0); //
      bitWrite(pll_in1[0], IN10_DVS, 1); // Programmable Divider divisor = 2
      bitWrite(pll_in2[0], IN20_GT0, 0); // IF counter mesurement period = 32ms
      bitWrite(pll_in2[0], IN20_GT1, 1); //
      bitWrite(pll_in2[1], IN21_DZ0, 1); // Dead zone = DZB
      bitWrite(pll_in2[1], IN21_DZ1, 0); //
      bitWrite(pll_in2[2], IN22_BO1, 0); // FM mode
      break;

    case YST93x_BAND_AM:
      band = YST93x_BAND_AM;
      bitWrite(pll_in1[0], IN10_R0,  0); // Reference frequency = 10kHz
      bitWrite(pll_in1[0], IN10_R3,  1); //
      bitWrite(pll_in1[0], IN10_DVS, 0); // Programmable Divider divisor = 1
      bitWrite(pll_in2[0], IN20_GT0, 1); // IF counter mesurement period = 8ms
      bitWrite(pll_in2[0], IN20_GT1, 0); //
      bitWrite(pll_in2[1], IN21_DZ0, 0); // Dead zone = DZC
      bitWrite(pll_in2[1], IN21_DZ1, 1); //
      bitWrite(pll_in2[2], IN22_BO1, 1); // AM mode
      break;
  }
  ccb.write(0x82, pll_in1, 3);
  ccb.write(0x92, pll_in2, 3);
}



uint8_t YST93xTune(uint16_t frequency) {
  uint16_t fpd = 0;
  uint8_t i = 0;
  uint8_t r[3];
  unsigned long IFCounter = 0;

  switch (band) {
    case YST93x_BAND_FM:
      // FM: fpd = (frequency + FI) / (50 * 2)
      fpd = (frequency + 107);
      break;

    case YST93x_BAND_AM:
      // AM: fpd = ((frequency + FI) / 10) << 4
      fpd = (frequency + 45) << 4;
      break;

    default: return 1;
  }

  YST93xSetMode(YST93x_MUTE);   // YST93x only injects FI signal into the PLL when in MUTE mode

  // Reset the IF counter and program the Frequency Programmable Divider (fpd)
  bitClear(pll_in1[0], IN10_CTE);
  pll_in1[1] = byte(fpd >> 8);
  pll_in1[2] = byte(fpd & 0x00ff);
  ccb.write(0x82, pll_in1, 3);

  // Start the IF counter
  bitSet(pll_in1[0], IN10_CTE);
  ccb.write(0x82, pll_in1, 3);

  // Wait for PLL to be locked (DO0_UL == 1)
  while (i < 20) {
    delay(50);
    ccb.read(0xa2, r, 3);  // Discard the 1st result: it is latched from the last count (as said on the datasheet)
    ccb.read(0xa2, r, 3);  // The 20 rightmost bits from r[0..2] are the IF counter result
    i = (bitRead(r[0], DO0_UL)) ? 100 : i + 1;
  }
  YST93xSetMode(YST93x_UNMUTE);   // Mute off / normal tuner mode
  return 0;
}


uint8_t YST93xIsStereo() {
  uint8_t r[3];

  ccb.read(0xa2, r, 3);
  return (bitRead(r[0], DO0_IN2) ? 0 : 1);
}

void setup() {
  delay(100);
  Serial.begin(9600);
  ccb.init();
  delay(100);
  YST93xInit();
  YST93xSetMode(YST93x_BAND_FM);
 // YST93xSetMode(YST93x_BAND_AM);
  int rr = analogRead(A7) / 3;
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = rr;
  }
  FMFrequency = 800 + rr;
  total = rr*numReadings;
  tuned = YST93xTune(FMFrequency);
/*  for(uint16_t qq = 52;qq<=172; qq++){
      tuned = YST93xTune(qq);
      Serial.println(qq);
      display.setBrightness(0x0f);
      display.showNumberDec(qq*10);
      delay(500);  
  }*/
//  tuned = YST93xTune(AMFrequency);
  display.setBrightness(0x0f);
  display.showNumberDec(FMFrequency);
  //Serial.println(tuned);
}

void loop() {
  
  total = total - readings[readIndex];
  readings[readIndex] = analogRead(A7) / 3;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  average = 800 + (total / numReadings);

  if (average != FMFrequency) {
    FMFrequency = average;
    tuned = YST93xTune(FMFrequency);
    display.clear();
    display.showNumberDec(FMFrequency);
    //delay(100);
  }
  delay(1);
  
}
