#define pinButtonSELECT                 3
#define pinButtonDOWN                   7
#define pinButtonUP                     12

#define pinDS18B20_IN                   2
#define pinDS18B20_OUT                  4

#include <EEPROM.h>
#include "microWire.h"
#include "GyverButton.h"
#include "microDS3231.h"
#include "microDS18B20.h"
#include "microLiquidCrystal_I2C.h"

GButton buttonSELECT(pinButtonSELECT);
GButton buttonDOWN(pinButtonDOWN);
GButton buttonUP(pinButtonUP);

MicroDS18B20 tempOUT(pinDS18B20_OUT);
MicroDS18B20 tempIN(pinDS18B20_IN);

MicroDS3231 rtc;

LiquidCrystal_I2C lcd(0x27, 20, 4);

//#########################################################
enum {
    SCREEN_TIME,
    SCREEN_TEMP,
    SCREEN_SET_TIME,
    SCREEN_SET_DATE,
    SCREEN_SET_YEAR,
    SCREEN_SET_ENABLE,
    SCREEN_SET_DISABLE,
    SCREEN_SET_TIME_BACKLIGHT,
    SCREEN_SET_TIME_CHANGE
};
//#########################################################
uint8_t LT[8] = { 0b00111, 0b01111, 0b11111, 0b11111, 0b11111,  0b11111, 0b11111, 0b11111 };
uint8_t UB[8] = { 0b11111, 0b11111, 0b11111, 0b00000, 0b00000,  0b00000, 0b00000, 0b00000 };
uint8_t RT[8] = { 0b11100, 0b11110, 0b11111, 0b11111, 0b11111,  0b11111, 0b11111, 0b11111 };
uint8_t LL[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,  0b11111, 0b01111, 0b00111 };
uint8_t LB[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000,  0b11111, 0b11111, 0b11111 };
uint8_t LR[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,  0b11111, 0b11110, 0b11100 };
uint8_t UMB[8] = { 0b11111, 0b11111, 0b11111, 0b00000, 0b00000,  0b00000, 0b11111, 0b11111 };
uint8_t LMB[8] = { 0b11111, 0b00000, 0b00000, 0b00000, 0b00000,  0b11111, 0b11111, 0b11111 };
//#########################################################
boolean isChange = true;
boolean isSettings = false;
uint8_t currentScreen = SCREEN_TIME;
unsigned long int lastTimeScreensUpdate = 0;
//#########################################################
struct cfg {
    uint8_t enableHours = 0;
    uint8_t enableMinutes = 0;
    uint8_t disableHours = 0;
    uint8_t disableMinutes = 0;
    uint8_t timeBacklight = 0;
    uint8_t timeChangeScreens = 0;
} config;

struct date {
    uint8_t day = 0;
    uint8_t month = 0;
    int year = 0;
    uint8_t seconds = 0;
    uint8_t minutes = 0;
    uint8_t hours = 0;
} currentDate;
//#########################################################

void setup(void) {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(4, 1);
    lcd.print("Loading...");
    //#########################################################
    buttonSELECT.setTickMode(AUTO);
    buttonDOWN.setTickMode(AUTO);
    buttonUP.setTickMode(AUTO);
    //#########################################################
    Serial.begin(9600);
    //#########################################################
    loadDataFromEEPROM();
    _createDisplaySymbols();
    //#########################################################
    lastTimeScreensUpdate = millis();
    //#########################################################
}

void loop(void) {
       
    if(millis() - lastTimeScreensUpdate > config.timeChangeScreens * 1000 && isSettings == false) {
        if(currentScreen == SCREEN_TIME) _showScreenTime();
        else if(currentScreen == SCREEN_TEMP) _showScreenTemp();
        lastTimeScreensUpdate = millis();
    }

    if(buttonSELECT.isTriple() && isSettings == false) {
        isSettings = true;
        currentScreen = SCREEN_SET_TIME;
    }

    if(isSettings) {
        if(currentScreen == SCREEN_SET_TIME) _showScreenSetTime();
        else if(currentScreen == SCREEN_SET_DATE) _showScreenSetDate();
        else if(currentScreen == SCREEN_SET_YEAR) _showScreenSetYear();
        else if(currentScreen == SCREEN_SET_ENABLE) _showScreenSetEnable();
        else if(currentScreen == SCREEN_SET_DISABLE) _showScreenSetDisable();
        else if(currentScreen == SCREEN_SET_TIME_BACKLIGHT) _showScreenSetTimeBacklight();
        else if(currentScreen == SCREEN_SET_TIME_CHANGE) _showScreenSetTimeChange();
    }
}

void _showScreenSetTimeChange(void) {
    if(isChange) {
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Option set time");
        lcd.setCursor(4, 1);
        lcd.print("change (s):");
        lcd.setCursor(9, 2);
        lcd.print(config.timeChangeScreens);
        isChange = false;
    }

    if(buttonSELECT.isDouble()) {
        isChange = true;
        isSettings = false;
        currentScreen = SCREEN_TIME;
        EEPROM.write(5, config.timeChangeScreens);
    }
    else if(buttonUP.isClick()) {
        isChange = true;
        if(config.timeChangeScreens + 1 <= 10) config.timeChangeScreens++;
        else if(config.timeChangeScreens + 1 > 10) config.timeChangeScreens = 1;
    }
}

void _showScreenSetTimeBacklight(void) {
    if(isChange) {
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Option set time");
        lcd.setCursor(3, 1);
        lcd.print("backlight (s):");
        lcd.setCursor(9, 2);
        lcd.print(config.timeBacklight);
        isChange = false;
    }

    if(buttonSELECT.isDouble()) {
        isChange = true;
        currentScreen = SCREEN_SET_TIME_CHANGE;
        EEPROM.write(4, config.timeBacklight);
    }
    else if(buttonUP.isClick()) {
        isChange = true;
        if(config.timeBacklight + 1 <= 10) config.timeBacklight++;
        else if(config.timeBacklight + 1 > 10) config.timeBacklight = 1;
    }
}

void _showScreenSetDisable(void) {
    if(isChange) {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Option set disable:");
        lcd.setCursor(7, 2);
        lcd.print(config.disableHours);
        lcd.print(":");
        lcd.print(config.disableMinutes);
        isChange = false;
    }

    if(buttonSELECT.isDouble()) {
        isChange = true;
        currentScreen = SCREEN_SET_TIME_BACKLIGHT;
        EEPROM.write(2, config.disableHours);
        EEPROM.write(3, config.disableMinutes);
    }
    else if(buttonDOWN.isClick()) {
        isChange = true;
        if(config.disableHours + 1 <= 23) config.disableHours++;
        else if(config.disableHours + 1 > 23) config.disableHours = 0;
    }
    else if(buttonUP.isClick()) {
        isChange = true;
        if(config.disableMinutes + 1 <= 59) config.disableMinutes++;
        else if(config.disableMinutes + 1 > 59) config.disableMinutes = 0;
    }
}

void _showScreenSetEnable(void) {
    if(isChange) {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Option set enable:");
        lcd.setCursor(7, 2);
        lcd.print(config.enableHours);
        lcd.print(":");
        lcd.print(config.enableMinutes);
        isChange = false;
    }

    if(buttonSELECT.isDouble()) {
        isChange = true;
        currentScreen = SCREEN_SET_DISABLE;
        EEPROM.write(0, config.enableHours);
        EEPROM.write(1, config.enableMinutes);
    }
    else if(buttonDOWN.isClick()) {
        isChange = true;
        if(config.enableHours + 1 <= 23) config.enableHours++;
        else if(config.enableHours + 1 > 23) config.enableHours = 0;
    }
    else if(buttonUP.isClick()) {
        isChange = true;
        if(config.enableMinutes + 1 <= 59) config.enableMinutes++;
        else if(config.enableMinutes + 1 > 59) config.enableMinutes = 0;
    }
}

void _showScreenSetYear(void) {
    if(isChange) {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Option set year:");
        lcd.setCursor(8, 2);
        lcd.print(currentDate.year);
        isChange = false;
    }

    if(buttonSELECT.isDouble()) {
        isChange = true;
        currentScreen = SCREEN_SET_ENABLE;
        rtc.setTime(currentDate.seconds, currentDate.minutes, currentDate.hours, currentDate.day, currentDate.month, currentDate.year);
    }
    else if(buttonUP.isClick()) {
        isChange = true;
        if(currentDate.year + 1 <= 2099) currentDate.year++;
        else if(currentDate.year + 1 > 2099) currentDate.year = 2018;
    }
}

void _showScreenSetDate(void) {
    if(isChange) {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Option set date:");
        lcd.setCursor(8, 2);
        lcd.print(currentDate.day);
        lcd.print("/");
        lcd.print(currentDate.month);
        isChange = false;
    }

    if(buttonSELECT.isDouble()) {
        isChange = true;
        currentScreen = SCREEN_SET_YEAR;
        rtc.setTime(currentDate.seconds, currentDate.minutes, currentDate.hours, currentDate.day, currentDate.month, currentDate.year);
    }
    else if(buttonDOWN.isClick()) {
        isChange = true;
        if(currentDate.day + 1 <= 31) currentDate.day++;
        else if(currentDate.day + 1 > 31) currentDate.day = 1;
    }
    else if(buttonUP.isClick()) {
        isChange = true;
        if(currentDate.month + 1 <= 12) currentDate.month++;
        else if(currentDate.month + 1 > 12) currentDate.month = 1;
    }
}

void _showScreenSetTime(void) {
    if(isChange) {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Option set time:");
        lcd.setCursor(7, 2);
        lcd.print(currentDate.hours);
        lcd.print(":");
        lcd.print(currentDate.minutes);
        isChange = false;
    }

    if(buttonSELECT.isDouble()) {
        isChange = true;
        currentScreen = SCREEN_SET_DATE;
        rtc.setTime(currentDate.seconds, currentDate.minutes, currentDate.hours, currentDate.day, currentDate.month, currentDate.year);
    }
    else if(buttonDOWN.isClick()) {
        isChange = true;
        if(currentDate.hours + 1 <= 23) currentDate.hours++;
        else if(currentDate.hours + 1 > 23) currentDate.hours = 0;
    }
    else if(buttonUP.isClick()) {
        isChange = true;
        if(currentDate.minutes + 1 <= 59) currentDate.minutes++;
        else if(currentDate.minutes + 1 > 59) currentDate.minutes = 0;
    }
}

void _showScreenTemp(void) {
    lcd.clear();
    tempOUT.requestTemp();
    tempIN.requestTemp();
    drawTemperature(tempOUT.getTemp(), 0, 0);
    drawTemperature(tempIN.getTemp(), 11, 2);

    currentScreen = SCREEN_TIME;
}

void _showScreenTime(void) {
    lcd.clear();
    getCurrentDate();
    drawNumber(currentDate.hours, 0, 0);
    _drawDots(7, 0);
    drawNumber(currentDate.minutes, 8, 0);

    lcd.setCursor(17, 0);
    lcd.print(getDayTitle(_getWeekNumberDay(currentDate.year, currentDate.month, currentDate.day)));

    drawNumber(currentDate.day, 5, 2);
    _drawDot(12, 3);
    drawNumber(currentDate.month, 13, 2);

    currentScreen = SCREEN_TEMP;
}

void loadDataFromEEPROM(void) {
    EEPROM.get(0, config.enableHours);
    EEPROM.get(1, config.enableMinutes);
    EEPROM.get(2, config.disableHours);
    EEPROM.get(3, config.disableMinutes);
    EEPROM.get(4, config.timeBacklight);
    EEPROM.get(5, config.timeChangeScreens);
    Serial.println("Time display enable is " + String(config.enableHours) + ":" + String(config.enableMinutes));
    Serial.println("Time display disable is " + String(config.disableHours) + ":" + String(config.disableMinutes));
    Serial.println("Time backlight display is " + String(config.timeBacklight) + " seconds");
    Serial.println("Time change screens is " + String(config.timeChangeScreens) + " seconds");
}

void getCurrentDate(void) {
    currentDate.day = rtc.getDate();
    currentDate.month = rtc.getMonth();
    currentDate.year = rtc.getYear();
    currentDate.seconds = rtc.getSeconds();
    currentDate.minutes = rtc.getMinutes();
    currentDate.hours = rtc.getHours();

    //Serial.println("Time is: " + String(currentDate.hours) + ":" + String(currentDate.minutes) + ":" + String(currentDate.seconds) + " " + String(currentDate.day) + "/" + String(currentDate.month) + "/" + String(currentDate.year));
}

void drawTemperature(int temp, uint8_t x, uint8_t y) {
    if(temp < 0) {
        _drawMinus(x, y);
        x += 2;
    }
    _drawSymbol((uint8_t)(abs(temp) / 10), x, y);
    _drawSymbol((uint8_t)(abs(temp) % 10), x + 4, y);
    lcd.setCursor(x + 4 + 3 + 1, y);
    lcd.write(223);
}

void drawNumber(int number, uint8_t x, uint8_t y) {
    if(number < 0) {
        _drawMinus(x, y);
        x += 2;
    }
    _drawSymbol((uint8_t)(abs(number) / 10), x, y);
    _drawSymbol((uint8_t)(abs(number) % 10), x + 4, y);
}

void _drawMinus(uint8_t x, uint8_t y) {
    lcd.setCursor(x, y);
    lcd.write(4);
}

String getDayTitle(uint8_t day) {
    if(day == 1) return "Mon";
    else if(day == 2) return "Tue";
    else if(day == 3) return "Wed";
    else if(day == 4) return "Thu";
    else if(day == 5) return "Fri";
    else if(day == 6) return "Sat";
    else if(day == 7) return "Sun";
}

void _drawDots(uint8_t x, uint8_t y) {
    lcd.setCursor(x, y);
    lcd.write(165);
    lcd.setCursor(x, y + 1);
    lcd.write(165);
}

void _drawDot(uint8_t x, uint8_t y) {
    lcd.setCursor(x, y);
    lcd.write(46);
}

void _drawSymbol(uint8_t digital, uint8_t x, uint8_t y) {
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
        lcd.setCursor(x, y + 1);
        lcd.write(4);
        lcd.write(4);
        lcd.write(5);
    }
    else if(digital == 10) {
        _drawSymbol(1, x, y);
        _drawSymbol(0, x + 4, y);
    }
}

void _createDisplaySymbols() {
    lcd.createChar(0, LT);
    lcd.createChar(1, UB);
    lcd.createChar(2, RT);
    lcd.createChar(3, LL);
    lcd.createChar(4, LB);
    lcd.createChar(5, LR);
    lcd.createChar(6, UMB);
    lcd.createChar(7, LMB);
}

const uint8_t daysInMonth [] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static uint16_t _getWeekNumberDay(uint16_t y, uint8_t m, uint8_t d) {
  if(y >= 2000) y -= 2000;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
  if(m > 2 && y % 4 == 0) ++days;
  return (days + 365 * y + (y + 3) / 4 - 1 + 6) % 7;
}