#include <EEPROM.h>
#include <Wire.h>
#include "GyverButton.h"
#include "microDS3231.h"
#include "microDS18B20.h"
#include <LiquidCrystal_I2C.h>

#define pinButtonSELECT                 3
#define pinButtonDOWN                   7
#define pinButtonUP                     12

#define pinDS18B20_IN                   2
#define pinDS18B20_OUT                  4

GButton buttonSELECT(pinButtonSELECT);
GButton buttonDOWN(pinButtonDOWN);
GButton buttonUP(pinButtonUP);

MicroDS18B20<pinDS18B20_IN> tempIN;
MicroDS18B20<pinDS18B20_OUT> tempOUT;

MicroDS3231 rtc;
LiquidCrystal_I2C display(0x27, 20, 4);

enum {
    SCREEN_TIME = 1,
    SCREEN_DATE,
    SCREEN_TEMP_IN,
    SCREEN_TEMP_OUT,

    SCREEN_TIME_SET,
    SCREEN_DATE_SET,
    SCREEN_YEAR_SET,
};

struct date {
    uint8_t d = 0;
    uint8_t m = 0;
    int y = 0;
    uint8_t ss = 0;
    uint8_t mm = 0;
    uint8_t hh = 0;
} currentDate;

uint8_t symbols[][8] = {
  { B00001, B00011, B00111, B01111, B11111, B11111, B11111, B11111 }, // 0 radius left top
  { B11111, B11111, B11111, B11111, B01111, B00111, B00011, B00001 }, // 1 radius left bottom
  { B10000, B11000, B11100, B11110, B11111, B11111, B11111, B11111 }, // 2 radius right top
  { B11111, B11111, B11111, B11111, B11110, B11100, B11000, B10000 }, // 3 radius right bottom
  { B11111, B11111, B11111, B00000, B00000, B00000, B00000, B00000 }, // 4 strip top
  { B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111 }, // 5 strip bottom
  { B00000, B01110, B10001, B10001, B01110, B00000, B00000, B00000 }, // 6 bubble
  { B00000, B00000, B01110, B01110, B01110, B00000, B00000, B00000 }, // 7 dot
};

boolean isSettings = false;
uint8_t currentScreen = SCREEN_TIME;
unsigned long int lastTimeScreenUpdate = 0;


void setup() {
  Serial.begin(9600);
  display.init();
  display.backlight();

  for(uint8_t i = 0; i < 8; i++) {
    display.createChar(i, symbols[i]);
  }
}

void loop() {

  if (millis() - lastTimeScreenUpdate >= 1.7 * 1000 && !isSettings) {
    display.clear();
    if (currentScreen == SCREEN_TIME) showScreenTime();
    else if(currentScreen == SCREEN_DATE) showScreenDate();
    else if(currentScreen == SCREEN_TEMP_IN) showScreenTempIn();
    else if(currentScreen == SCREEN_TEMP_OUT) showScreenTempOut();

    currentScreen += 1;
    if (currentScreen > 4) currentScreen = SCREEN_TIME;

    lastTimeScreenUpdate = millis();
  }
  
  if (millis() - lastTimeScreenUpdate >= 0.8 * 1000 && isSettings) {
    display.clear();
    
    if (currentScreen == SCREEN_TIME_SET) showScreenTimeSet();
    else if (currentScreen == SCREEN_DATE_SET) showScreenDateSet();
    else if (currentScreen == SCREEN_YEAR_SET) showScreenYearSet();
    
    lastTimeScreenUpdate = millis();
  }
  
  buttonSELECT.tick();
  buttonDOWN.tick();
  buttonUP.tick();

  if (isSettings) {

    if (currentScreen == SCREEN_TIME_SET) {
      if (buttonDOWN.isClick()) {
        if(currentDate.hh + 1 <= 23) currentDate.hh++;
        else if(currentDate.hh + 1 > 23) currentDate.hh = 0;
      }
      if (buttonUP.isClick()) {
        if(currentDate.mm + 1 <= 59) currentDate.mm++;
        else if(currentDate.mm + 1 > 59) currentDate.mm = 0;
      }
      if (buttonSELECT.isDouble()) currentScreen += 1;
    }
    else if (currentScreen == SCREEN_DATE_SET) {
      if (buttonDOWN.isClick()) {
        if(currentDate.d + 1 <= 31) currentDate.d++;
        else if(currentDate.d + 1 > 31) currentDate.d = 1;
      }
      if (buttonUP.isClick()) {
        if(currentDate.m + 1 <= 12) currentDate.m++;
        else if(currentDate.m + 1 > 12) currentDate.m = 1;
      }
      if (buttonSELECT.isDouble()) currentScreen += 1;
    }
    else if (currentScreen == SCREEN_YEAR_SET) {
      if (buttonDOWN.isClick()) currentDate.y -= 1;
      if (buttonUP.isClick()) currentDate.y += 1;
      if (buttonSELECT.isDouble()) {
        rtc.setTime(0, currentDate.mm, currentDate.hh, currentDate.d, currentDate.m, currentDate.y);
        currentScreen = SCREEN_TIME;
        isSettings = false;
      }
    }
  }
  else {
    if (buttonSELECT.isSingle())
      currentScreen = SCREEN_TEMP_IN;
    if (buttonSELECT.isTriple()) {
      currentScreen = SCREEN_TIME_SET;
      isSettings = true;
    }
  }
}

void showScreenYearSet() {
  printNumber(currentDate.y % 100, 0);
  printLabel();
}

void showScreenDateSet() {
  printNumber(currentDate.d, 0);
  printNumber(currentDate.m, 11);
  printDot(10, 3);
  printLabel();
}

void showScreenTimeSet() {
  printNumber(currentDate.hh, 0);
  printNumber(currentDate.mm, 11);
  printDot(10, 1);
  printDot(10, 2);
  printLabel();
}

void showScreenTempIn() {
  tempIN.requestTemp();
  display.setCursor(1, 1);
  display.print("BHYTPU");
  printNumber(tempIN.getTemp(), 9);
}

void showScreenTempOut() {
  tempOUT.requestTemp();
  display.setCursor(1, 1);
  display.print("BHE");
  if (tempOUT.readTemp())
    printNumber(tempOUT.getTemp(), 9);
  else
    printNumber(-99, 9);
}

void showScreenDate() {
  getCurrentDate();
  printNumber(currentDate.d, 0);
  printNumber(currentDate.m, 11);
  printDot(10, 3);
}

void showScreenTime() {
  getCurrentDate();
  printNumber(currentDate.hh, 0);
  printNumber(currentDate.mm, 11);
  printDot(10, 1);
  printDot(10, 2);
}

void getCurrentDate() {
    currentDate.d = rtc.getDate();
    currentDate.m = rtc.getMonth();
    currentDate.y = rtc.getYear();
    currentDate.ss = rtc.getSeconds();
    currentDate.mm = rtc.getMinutes();
    currentDate.hh = rtc.getHours();
}

void printLabel() {
  display.setCursor(9, 0);
  display.print("S");
  display.setCursor(9, 1);
  display.print("E");
  display.setCursor(9, 2);
  display.print("T");
  display.setCursor(9, 3);
  display.print(":");
}

void printNumber(int number, uint8_t x) {
    if(number < 0) {
        printMinus(x, 2);
        printMinus(x - 1, 2);
        x += 2;
    }
    printSymbol((uint8_t)(abs(number) / 10), x, 0);
    printSymbol((uint8_t)(abs(number) % 10), x + 5, 0);
}

void printMinus(uint8_t x, uint8_t y) {
    display.setCursor(x, y);
    display.write(4);
}

void printDot(uint8_t x, uint8_t y) {
  display.setCursor(x, y);
  display.write(7);
}

void printSymbol(uint8_t digital, uint8_t x, uint8_t y) {
    if(digital == 0) {
      printSymbol(8, x, y);
      display.setCursor(x + 1, y + 2);
      display.print("  ");
    }
    else if (digital == 1) {
      display.setCursor(x + 3, y);
      display.write(2);
      display.setCursor(x + 3, y + 1);
      display.write(3);
      display.setCursor(x + 3, y + 2);
      display.write(2);
      display.setCursor(x + 3, y + 3);
      display.write(3);
    }
    else if (digital == 2) {
      printSymbol(8, x, y);
      display.setCursor(x, y + 1);
      display.print(" ");
      display.setCursor(x + 3, y + 2);
      display.print(" ");
    }
    else if (digital == 3) {
      printSymbol(8, x, y);
      display.setCursor(x, y + 1);
      display.print(" ");
      display.setCursor(x, y + 2);
      display.print(" ");
    }
    else if (digital == 4) {
      display.setCursor(x, y);
      display.write(0);
      display.setCursor(x + 3, y);
      display.write(2);
      display.setCursor(x, y + 1);
      display.write(1);
      display.setCursor(x + 3, y + 1);
      display.write(3);
      display.setCursor(x + 1, y + 2);
      display.write(4);
      display.write(4);
      display.write(2);
      display.setCursor(x + 3, y + 3);
      display.write(3);
    }
    else if (digital == 5) {
      printSymbol(6, x, y);
      display.setCursor(x, y + 2);
      display.print(" ");
    }
    else if (digital == 6) {
      printSymbol(8, x, y);
      display.setCursor(x + 3, y + 1);
      display.print(" ");
    }
    else if (digital == 7) {
      display.setCursor(x, y);
      display.write(0);
      display.write(4);
      display.write(4);
      display.write(2);
      display.setCursor(x + 3, y + 1);
      display.write(3);
      display.setCursor(x + 3, y + 2);
      display.write(2);
      display.setCursor(x + 3, y + 3);
      display.write(3);
    }
    else if (digital == 8) {
      display.setCursor(x, y);
      display.write(0);
      display.write(4);
      display.write(4);
      display.write(2);
      display.setCursor(x, y + 1);
      display.write(1);
      display.setCursor(x + 3, y + 1);
      display.write(3);
      display.setCursor(x, y + 2);
      display.write(0);
      display.write(4);
      display.write(4);
      display.write(2);
      display.setCursor(x, y + 3);
      display.write(1);
      display.write(5);
      display.write(5);
      display.write(3);
    }
    else if (digital == 9) {
      printSymbol(8, x, y);
      display.setCursor(x, y + 2);
      display.print(" ");
    }
    else if (digital == 0) {
      printSymbol(8, x, y);
      display.setCursor(x + 1, y + 2);
      display.print("  ");
    }
}