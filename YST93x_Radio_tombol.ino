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

const int st[] = {
  877,// "HardRock",
 // 881,// "StudioEast",
  885,// "MoraFM",
 // 889,// "Auto",
 // 893,// "ElShinta",
 // 897,// "Global",
  901,// "Urban Bdg",
 // 905,// "Cakra",
  909,// "X - Channel",
//  913,// "Trijaya", //1
  917,// "Voks",
 // 921,// "SuaraIndh",
  925,// "Maestro",
  933,// "Sonora",
 // 937,// "Paramuda",
 // 944,// "DeltaFM",
 // 948,// "FitFM",
  952,// "Fajri",
 // 956,// "B - Radio",
  960,// "RRI Pro2",
 // 964,// "PersibFM", //2
  968,// "Kharisma",
 // 972,// "NewShinta",
  976,// "RRI Pro1",
 // 980,// "MayaFM",
  984,// "Prambors",
  988,// "Raka",
 // 992,// "Mom&Kids",
 // 996,// "Unasko",
  1000,// "99ers",
 // 1004,// "KLCBS",
 // 1008,// "Kandaga", //3
 // 1011,// "MGT",
 // 1015,// "Dahlia",
  1019,// "Cosmo",
  1023,// "RaseFM",
  1027,// "MQFM",
 // 1031,// "OzRadio",
 // 1035,// "Chevy",
 // 1039,// "UNIKOM",
  1043,// "RodjaFM",
 // 1047,// "RamaFM", //4
  1051,// "I - Radio",
 // 1055,// "Garuda",
  1059,// "Ardan",
 // 1063,// "UrbanFM",
 // 1067,// "Mara FM",
 // 1071,// "K - lite",
 // 1075,// "PRFM",
 // 1080,// "Arduino"
};

const uint8_t FM1[] = {
  SEG_G,
  SEG_A | SEG_E | SEG_F | SEG_G,
  SEG_B | SEG_C,
  SEG_G
};
const uint8_t FM2[] = {
  SEG_G,
  SEG_A | SEG_E | SEG_F | SEG_G,
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
  SEG_G
};
const uint8_t AM[] = {
  SEG_G,
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
  SEG_B | SEG_C,
  SEG_G
};

const int CLK = 7; //Set the CLK pin connection to the display
const int DIO = 8; //Set the DIO pin connection to the display

const int up = 9; //pin up
const int down = 10; //pin up
const int menu = 11; //pin up
int jmlst = sizeof(st) / sizeof(int);
int i = 4;

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.
SanyoCCB ccb(3, 2, 4, 5); // Pins: DO CL DI CE

byte pll_in1[3];
byte pll_in2[3];

// Initial frequencies
uint16_t FMFrequency = 937;   // MHz * 10
//uint16_t AMFrequency = 103;    // KHZ / 10
uint16_t AMFrequency = 1035;    // KHZ / 10

uint8_t band = YST93x_BAND_FM;
uint8_t tuned = 0;

int steppp = 1, mode = 0;
float amsteppp = 9;

void YST93xInit() {
  memset(pll_in1, 0, 3);
  memset(pll_in2, 0, 3);
  bitSet(pll_in2[0], IN20_IFS);   // IF counter in normal mode
  bitSet(pll_in2[1], IN21_UL0);   // Phase error detection width = 0us
  bitSet(pll_in2[2], IN22_BO2);   // Mute off / normal tuner mode
  bitSet(pll_in2[2], IN22_BO4);
  //tambahan
  /*bitSet(pll_in2[2], IN22_IOC1);
  bitSet(pll_in2[2], IN22_IOC2);
  bitSet(pll_in2[2], IN22_IO2);   // Mute off / normal tuner mode
  bitSet(pll_in2[2], IN22_IO1);*/
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
      //asli
      /*
        bitWrite(pll_in1[0], IN10_R0,  1); // Reference frequency = 50kHz
        bitWrite(pll_in1[0], IN10_R3,  0); //
        bitWrite(pll_in1[0], IN10_DVS, 1); // Programmable Divider divisor = 2
        bitWrite(pll_in2[0], IN20_GT0, 0); // IF counter mesurement period = 32ms
        bitWrite(pll_in2[0], IN20_GT1, 1); //
        bitWrite(pll_in2[1], IN21_DZ0, 1); // Dead zone = DZB
        bitWrite(pll_in2[1], IN21_DZ1, 0); //
        bitWrite(pll_in2[2], IN22_BO1, 0); // FM mode
      */

      bitWrite(pll_in1[0], IN10_R0,  1); // Reference frequency = 50kHz
      bitWrite(pll_in1[0], IN10_R3,  0); //
      bitWrite(pll_in1[0], IN10_DVS, 1); // Programmable Divider divisor = 2
      bitWrite(pll_in2[0], IN20_GT0, 0); // IF counter mesurement period = 32ms
      bitWrite(pll_in2[0], IN20_GT1, 1); //
      bitWrite(pll_in2[1], IN21_DZ0, 1); // Dead zone = DZB
      bitWrite(pll_in2[1], IN21_DZ1, 0); //
      bitWrite(pll_in2[2], IN22_BO1, 0); // FM mode
      bitWrite(pll_in2[2], IN22_IO1, 0);
      bitWrite(pll_in2[2], IN22_BO2, 1); // AM mode out1 на микросхеме
      break;

    case YST93x_BAND_AM:
      band = YST93x_BAND_AM;
      bitWrite(pll_in1[0], IN10_R0,  1); // Reference frequency = 9khz
      bitWrite(pll_in1[0], IN10_R3,  1); //
      bitWrite(pll_in1[0], IN10_DVS, 0); // Programmable Divider divisor = 1
      bitWrite(pll_in1[0], IN10_SNS, 0); // Programmable Divider divisor = 1
      bitWrite(pll_in2[0], IN20_GT0, 1); // IF counter mesurement period = 8ms
      bitWrite(pll_in2[0], IN20_GT1, 0); //
      bitWrite(pll_in2[1], IN21_DZ0, 0); // Dead zone = DZC
      bitWrite(pll_in2[1], IN21_DZ1, 1); //
      bitWrite(pll_in2[2], IN22_BO1, 1); // AM mode
      bitWrite(pll_in2[2], IN22_IO1, 1);
      bitWrite(pll_in2[2], IN22_BO2, 0); // AM mode out1 на микросхеме

      


      /*
        //asli
        bitWrite(pll_in1[0], IN10_R0,  0); // Reference frequency = 10kHz
        bitWrite(pll_in1[0], IN10_R3,  1); //
        bitWrite(pll_in1[0], IN10_DVS, 0); // Programmable Divider divisor = 1
        bitWrite(pll_in2[0], IN20_GT0, 1); // IF counter mesurement period = 8ms
        bitWrite(pll_in2[0], IN20_GT1, 0); //
        bitWrite(pll_in2[1], IN21_DZ0, 0); // Dead zone = DZC
        bitWrite(pll_in2[1], IN21_DZ1, 1); //
        bitWrite(pll_in2[2], IN22_BO1, 1); // AM mode
      */
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
      fpd = ((frequency + 455)/9) << 4;
      //      fpd = (frequency + 45) << 4;
      break;

    default: return 1;
  }

  YST93xSetMode(YST93x_MUTE);   // YST93x only injects FI signal into the PLL when in MUTE mode

  // Reset the IF counter and program the Frequency Programmable Divider (fpd)
  Serial.print("fpd = "); Serial.println(fpd);
  bitClear(pll_in1[0], IN10_CTE);
  pll_in1[1] = byte(fpd >> 8);
  pll_in1[2] = byte(fpd & 0x00ff);
  ccb.write(0x82, pll_in1, 3);

  // Start the IF counter
  bitSet(pll_in1[0], IN10_CTE);
  ccb.write(0x82, pll_in1, 3);

  // Wait for PLL to be locked (DO0_UL == 1)
  /*
    while (i < 20) {
    delay(50);
    if (band == YST93x_BAND_FM) {
      ccb.read(0xa2, r, 3);  // Discard the 1st result: it is latched from the last count (as said on the datasheet)
      ccb.read(0xa2, r, 3);  // The 20 rightmost bits from r[0..2] are the IF counter result
      i = (bitRead(r[0], DO0_UL)) ? 100 : i + 1;
    } else if (band == YST93x_BAND_AM) {
      ccb.read(0xb2, r, 3);  // Discard the 1st result: it is latched from the last count (as said on the datasheet)
      ccb.read(0xb2, r, 3);  // The 20 rightmost bits from r[0..2] are the IF counter result
      i = (bitRead(r[0], DO0_UL)) ? 100 : i + 1;
    }


    }*/
  YST93xSetMode(YST93x_UNMUTE);   // Mute off / normal tuner mode
  return 0;
}


uint8_t YST93xIsStereo() {
  uint8_t r[3];

  ccb.read(0xa2, r, 3);
  return (bitRead(r[0], DO0_IN2) ? 0 : 1);
}

void setup() {
  delay(50);
  Serial.begin(9600);
  Serial.print("jml : ");
  Serial.println(jmlst);
  pinMode(up, INPUT);
  pinMode(down, INPUT);
  pinMode(menu, INPUT);
  ccb.init();
  delay(50);
  YST93xInit();

  mode = 0;
  //  YST93xSetMode(YST93x_BAND_AM);
  //  tuned = YST93xTune(AMFrequency);
  // display.setBrightness(0x0f);
  //  display.showNumberDec(AMFrequency);

  YST93xSetMode(YST93x_BAND_FM);
  tuned = YST93xTune(st[i]);
  display.setBrightness(0x0f);
  display.setSegments(FM1);
  delay(500);
  display.showNumberDec(st[i]);
}

void loop() {
  if (digitalRead(up) == HIGH) {
    if (mode == 0) {
      i = i + 1;
      if (i == jmlst)i = 0;
      tuned = YST93xTune(st[i]);
      display.showNumberDec(st[i]);
      FMFrequency = st[i];
    } else if (mode == 1) {
      FMFrequency += steppp;
      if (FMFrequency > 1080)FMFrequency = 600;
      tuned = YST93xTune(FMFrequency);
      display.showNumberDec(FMFrequency);
    } else if (mode == 2) {
      AMFrequency += amsteppp;
      if (AMFrequency > 1800)AMFrequency = 531;
      tuned = YST93xTune(AMFrequency);
      display.showNumberDec(AMFrequency);
    }
  } else if (digitalRead(down) == HIGH) {
    if (mode == 0) {
      i = i - 1;
      if (i == -1)i = (jmlst - 1);
      tuned = YST93xTune(st[i]);
      display.showNumberDec(st[i]);
      FMFrequency = st[i];
    } else if (mode == 1) {
      FMFrequency -= steppp;
      if (FMFrequency < 600)FMFrequency = 1080;
      tuned = YST93xTune(FMFrequency);
      display.showNumberDec(FMFrequency);
    } else if (mode == 2) {
      AMFrequency -= amsteppp;
      if (AMFrequency < 530)AMFrequency = 1800;
      tuned = YST93xTune(AMFrequency);
      display.showNumberDec(AMFrequency);

    }
  } else if (digitalRead(menu) == HIGH) {
    mode = mode + 1;
    if (mode == 3)mode = 0;
    if (mode == 0) {
      display.setSegments(FM1);
      delay(500);
      YST93xSetMode(YST93x_BAND_FM);
      tuned = YST93xTune(st[i]);
      display.showNumberDec(st[i]);
    } else if (mode == 1) {
      display.setSegments(FM2);
      delay(500);
      YST93xSetMode(YST93x_BAND_FM);
      tuned = YST93xTune(FMFrequency);
      display.showNumberDec(FMFrequency);
    } else if (mode == 2) {
      display.setSegments(AM);
      delay(500);
      YST93xSetMode(YST93x_BAND_AM);
      tuned = YST93xTune(AMFrequency);
      display.showNumberDec(AMFrequency);
    }
  }
  delay(100);
}
