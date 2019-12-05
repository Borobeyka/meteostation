#define pinButtonUP         12
#define pinButtonDOWN       7
#define pinButtonSELECT     3

#include <EEPROM.h>
#include "GyverButton.h"
#include "microDS3231.h"
#include "microDS18B20.h"
#include "microLiquidCrystal_I2C.h"

MicroDS3231 rtc;
GButton buttonUP(pinButtonUP);
GButton buttonDOWN(pinButtonDOWN);
GButton buttonSELECT(pinButtonSELECT);
LiquidCrystal_I2C lcd(0x27, 20, 4);

boolean showSettings = false;
uint8_t displayEnableHours = 0,
        displayEnableMinutes = 0,
        displayDisableHours = 0,
        displayDisableMinutes = 0,
        displayTimeBacklight = 0;
struct dateStruct {
    uint8_t seconds = 0;
    uint8_t minutes = 0;
    uint8_t hours = 0;
    uint8_t day = 0;
    uint8_t month = 0;
    int year = 0;
} currentDate;
// цифры
uint8_t LT[8] = { 0b00111, 0b01111, 0b11111, 0b11111, 0b11111,  0b11111, 0b11111, 0b11111 };
uint8_t UB[8] = { 0b11111, 0b11111, 0b11111, 0b00000, 0b00000,  0b00000, 0b00000, 0b00000 };
uint8_t RT[8] = { 0b11100, 0b11110, 0b11111, 0b11111, 0b11111,  0b11111, 0b11111, 0b11111 };
uint8_t LL[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,  0b11111, 0b01111, 0b00111 };
uint8_t LB[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000,  0b11111, 0b11111, 0b11111 };
uint8_t LR[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,  0b11111, 0b11110, 0b11100 };
uint8_t UMB[8] = { 0b11111, 0b11111, 0b11111, 0b00000, 0b00000,  0b00000, 0b11111, 0b11111 };
uint8_t LMB[8] = { 0b11111, 0b00000, 0b00000, 0b00000, 0b00000,  0b11111, 0b11111, 0b11111 };

void setup(void) {
    lcd.init();
    buttonUP.setTickMode(AUTO);
    buttonDOWN.setTickMode(AUTO);
    buttonSELECT.setTickMode(AUTO);
    lcd.backlight();

    Serial.begin(9600);

    //loadDataFromEEPROM();
    loadClock();
    drawClock(10, 34, 2, 0, true);
}

void loop(void) {
    
    if(buttonSELECT.isTriple() && showSettings == false) {
        getCurrentDate();
        showSettingsScreen();
    }
    else if(buttonSELECT.isTriple() && showSettings == true) showMainScreen();
}

void showSettingsScreen(void) {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("SETTINGS");
    lcd.setCursor(5, 1);
    lcd.print(String(currentDate.day) + "." + String(currentDate.month) + "." + String(currentDate.year));
}
/*
  04/12/2019
    21:53
*/
void showMainScreen(void) {
    
}

void getCurrentDate(void) {
    currentDate.seconds = rtc.getSeconds();
    currentDate.minutes = rtc.getMinutes();
    currentDate.hours = rtc.getHours();
    currentDate.day = rtc.getDate();
    currentDate.month = rtc.getMonth();
    currentDate.year = rtc.getYear();
}

void loadDataFromEEPROM(void) {
    displayEnableHours = EEPROM.read(0);
    displayEnableMinutes = EEPROM.read(1);
    displayDisableHours = EEPROM.read(2);
    displayDisableMinutes = EEPROM.read(3);
    displayTimeBacklight = EEPROM.read(4);
    Serial.println("Display enable before " + String(displayEnableHours) + ":" + String(displayEnableMinutes));
    Serial.println("Display disable before " + String(displayDisableHours) + ":" + String(displayDisableMinutes));
    Serial.println("Display time backlight is " + String(displayTimeBacklight) + " seconds");
}

void drawClock(byte hours, byte minutes, byte x, byte y, boolean dots) {
    lcd.clear();

    if (hours / 10 == 0) drawSymbol(10, x, y);
    else drawSymbol(hours / 10, x, y);
    drawSymbol(hours % 10, x + 4, y);
    uint8_t currentX = x + 4;
    if(dots)
    {
        drawdots(x + 7, y, true);
        drawSymbol(minutes / 10, x + 8, y);
        drawSymbol(minutes % 10, x + 12, y);
    }
}

void drawdots(byte x, byte y, boolean state) {
    byte code;
    if (state) code = 165;
    else code = 32;
    lcd.setCursor(x, y);
    lcd.write(code);
    lcd.setCursor(x, y + 1);
    lcd.write(code);
}

void drawSymbol(byte digital, byte x, byte y) {
    if(digital == 0) {
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(1);
        lcd.write(2);
        lcd.setCursor(x, y + 1);
        lcd.write(3);
        lcd.write(4);
        lcd.write(5);
    }
    else if(digital == 1) {
        lcd.setCursor(x + 2, y);
        lcd.write(2);
        lcd.setCursor(x + 2, y + 1);
        lcd.write(5);
    }
    else if(digital == 2) {
        lcd.setCursor(x, y);
        lcd.write(6);
        lcd.write(6);
        lcd.write(2);
        lcd.setCursor(x, y + 1);
        lcd.write(3);
        lcd.write(7);
        lcd.write(7);
    }
    else if(digital == 3) {
        lcd.setCursor(x, y);
        lcd.write(6);
        lcd.write(6);
        lcd.write(2);
        lcd.setCursor(x, y + 1);
        lcd.write(7);
        lcd.write(7);
        lcd.write(5);
    }
    else if(digital == 4) {
        lcd.setCursor(x, y);
        lcd.write(3);
        lcd.write(4);
        lcd.write(2);
        lcd.setCursor(x + 2, y + 1);
        lcd.write(5);
    }
    else if(digital == 5) {
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(6);
        lcd.write(6);
        lcd.setCursor(x, y + 1);
        lcd.write(7);
        lcd.write(7);
        lcd.write(5);
    }
    else if(digital == 6) {
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(6);
        lcd.write(6);
        lcd.setCursor(x, y + 1);
        lcd.write(3);
        lcd.write(7);
        lcd.write(5);
    }
    else if(digital == 7) {
        lcd.setCursor(x, y);
        lcd.write(1);
        lcd.write(1);
        lcd.write(2);
        lcd.setCursor(x + 1, y + 1);
        lcd.write(0);
    }
    else if(digital == 8) {
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(6);
        lcd.write(2);
        lcd.setCursor(x, y + 1);
        lcd.write(3);
        lcd.write(7);
        lcd.write(5);
    }
    else if(digital == 9) {
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(6);
        lcd.write(2);
        lcd.setCursor(x + 1, y + 1);
        lcd.write(4);
        lcd.write(5);
    }
    else if(digital == 10) {
        lcd.setCursor(x, y);
        lcd.write(32);
        lcd.write(32);
        lcd.write(32);
        lcd.setCursor(x, y + 1);
        lcd.write(32);
        lcd.write(32);
        lcd.write(32);
    }
}

void loadClock() {
    lcd.createChar(0, LT);
    lcd.createChar(1, UB);
    lcd.createChar(2, RT);
    lcd.createChar(3, LL);
    lcd.createChar(4, LB);
    lcd.createChar(5, LR);
    lcd.createChar(6, UMB);
    lcd.createChar(7, LMB);
}

/*
void printTime() {
    Serial.print(rtc.getHours());
    Serial.print(":");
    Serial.print(rtc.getMinutes());
    Serial.print(":");
    Serial.print(rtc.getSeconds());
    Serial.print(" ");
    Serial.print(rtc.getDay());
    Serial.print(" ");
    Serial.print(rtc.getDate());
    Serial.print("/");
    Serial.print(rtc.getMonth());
    Serial.print("/");
    Serial.println(rtc.getYear());
}
*/
