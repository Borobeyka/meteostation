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
struct dateStruct {
    uint8_t seconds = 0;
    uint8_t minutes = 0;
    uint8_t hours = 0;
    uint8_t day = 0;
    uint8_t month = 0;
    int year = 0;
    String dayTitle = "";
} currentDate;

struct config {
    uint8_t enableHours = 0;
    uint8_t enableMinutes = 0;
    uint8_t disableHours = 0;
    uint8_t disableMinutes = 0;
    uint8_t timeBacklight = 0;
    uint8_t timeUpdateScreen = 0;
} cfg;

enum {
    SCREEN_TIME,
    SCREEN_TEMP
};

uint8_t LT[8] = { 0b00111, 0b01111, 0b11111, 0b11111, 0b11111,  0b11111, 0b11111, 0b11111 };
uint8_t UB[8] = { 0b11111, 0b11111, 0b11111, 0b00000, 0b00000,  0b00000, 0b00000, 0b00000 };
uint8_t RT[8] = { 0b11100, 0b11110, 0b11111, 0b11111, 0b11111,  0b11111, 0b11111, 0b11111 };
uint8_t LL[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,  0b11111, 0b01111, 0b00111 };
uint8_t LB[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000,  0b11111, 0b11111, 0b11111 };
uint8_t LR[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,  0b11111, 0b11110, 0b11100 };
uint8_t UMB[8] = { 0b11111, 0b11111, 0b11111, 0b00000, 0b00000,  0b00000, 0b11111, 0b11111 };
uint8_t LMB[8] = { 0b11111, 0b00000, 0b00000, 0b00000, 0b00000,  0b11111, 0b11111, 0b11111 };

unsigned long int lastDisplayUpdate = 0;
uint8_t currentScreen = SCREEN_TIME;
unsigned long int timeBlinkDots = 0;

void setup(void) {
    lcd.init();
    //loadDataFromEEPROM();
    createDisplaySymbols();
    buttonUP.setTickMode(AUTO);
    buttonDOWN.setTickMode(AUTO);
    buttonSELECT.setTickMode(AUTO);
    lcd.backlight();
    lastDisplayUpdate = millis();

    Serial.begin(9600);
}

void loop(void) {
    if(buttonSELECT.isTriple() && showSettings == true || showSettings == false) {
        if(millis() - lastDisplayUpdate >= 3 * 1000) { // cfg.timeUpdateScreen
            if(currentScreen == SCREEN_TIME) {
                drawScreenTime();
                //currentScreen = SCREEN_TEMP;
            }
            else if (currentScreen == SCREEN_TEMP) {
                currentScreen = SCREEN_TIME;
            }
            lastDisplayUpdate = millis();
        }
    }
}

void drawScreenTime(void) {
    lcd.clear();
    getCurrentDate();
    drawNumber(currentDate.hours, 0, 0);
    drawDots(7, 0);
    drawNumber(currentDate.minutes, 8, 0);

    lcd.setCursor(16, 0);
    lcd.print(getDayTitle(getWeekDay(currentDate.year, currentDate.month, currentDate.day)));

    Serial.println(String(currentDate.day) + "/" + String(currentDate.month) + "/" + String(currentDate.year % 100));

    drawNumber(currentDate.day, 5, 2);
    drawDot(12, 3);
    drawNumber(currentDate.month, 13, 2);
}

void loadDataFromEEPROM(void) {
    cfg.enableHours = EEPROM.read(0);
    cfg.enableMinutes = EEPROM.read(1);
    cfg.disableHours = EEPROM.read(2);
    cfg.disableMinutes = EEPROM.read(3);
    cfg.timeBacklight = EEPROM.read(4);
    cfg.timeUpdateScreen = EEPROM.read(5);
    Serial.println("Display enable before " + String(cfg.enableHours) + ":" + String(cfg.enableMinutes));
    Serial.println("Display disable before " + String(cfg.disableHours) + ":" + String(cfg.disableMinutes));
    Serial.println("Display time backlight is " + String(cfg.timeBacklight) + "(s)");
    Serial.println("Screen update time is " + String(cfg.timeUpdateScreen) + "(s)");
}

void drawNumber(uint8_t number, uint8_t x, uint8_t y) {
    drawSymbol(number / 10, x, y);
    drawSymbol(number % 10, x + 4, y);
}

void getCurrentDate(void) {
    currentDate.seconds = rtc.getSeconds();
    currentDate.minutes = rtc.getMinutes();
    currentDate.hours = rtc.getHours();
    currentDate.day = rtc.getDate();
    currentDate.month = rtc.getMonth();
    currentDate.year = rtc.getYear();
    currentDate.dayTitle = getDayTitle(getWeekDay(currentDate.year, currentDate.month, currentDate.day));
}

String getDayTitle(uint8_t day) {
    if(day == 1) return "Pon"; //return "Пон";
    else if(day == 2) return "Vtor"; //return "Втор";
    else if(day == 3) return "Sred"; //return "Сред";
    else if(day == 4) return "Chet"; //return "Четв";
    else if(day == 5) return "Patn"; //return "Пятн";
    else if(day == 6) return "Subb"; //return "Субб";
    else if(day == 7) return "Vskr"; //return "Вскр";
}

void drawDots(uint8_t x, uint8_t y) {
    lcd.setCursor(x, y);
    lcd.write(165);
    lcd.setCursor(x, y + 1);
    lcd.write(165);
}

void drawDot(uint8_t x, uint8_t y) {
    lcd.setCursor(x, y);
    lcd.write(46);
}

void drawSymbol(uint8_t digital, uint8_t x, uint8_t y) {
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
        drawSymbol(1, x, y);
        drawSymbol(0, x + 4, y);
    }
}

void createDisplaySymbols() {
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
static uint16_t getWeekDay(uint16_t y, uint8_t m, uint8_t d) {
  if (y >= 2000)
    y -= 2000;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
  if (m > 2 && y % 4 == 0)
    ++days;
  //return days + 365 * y + (y + 3) / 4 - 1;
  return (days + 365 * y + (y + 3) / 4 - 1 + 6) % 7;
}