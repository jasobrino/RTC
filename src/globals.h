
#ifndef GLOBALS_H
#define GLOBALS_H

// #include <Arduino.h>
#include <Ticker.h>
#include <RTClib.h>
#include <DHT.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

struct st_hora {
    byte hora;
    byte min;
    byte seg;
};
struct st_fecha {
    int anyo;
    byte mes;
    byte dia;
};

// cabecera de funciones
void ICACHE_RAM_ATTR handleInterrupt();
// funciones en RTC.cpp
// void mostrarHora();
void updateTempHum();
void updateTime();
void grabarHora(byte h, byte mi, byte s);
void grabarFecha(int a, byte m, byte d);
void setUpdateTempHum();
void updateTempHum();

//funciones en oled.hpp
// void OledInit();
// void oledVerHora(st_hora h);
// void oledVerTemp(float t, float h);
// extern U8X8_SH1106_128X64_NONAME_HW_I2C u8x8;

// funciones en epaper.cpp
void epaper_init();
void epaperMostrarDatos();


extern uint8_t pulsador;
extern uint8_t DHT_pin;
extern bool modoSleep;
extern bool restServer;

// extern GxEPD_Class display;
extern GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display;

// creamos los objetos del timer, el RTC y el sensor DHT22
extern RTC_DS3231 rtc;
extern DHT dht;

//  variables globales
extern st_hora stHora, stAlarm1;
extern st_fecha stFecha;

extern uint8_t dayOfWeek;
extern float temp, hum;
extern bool horaConsola;
extern volatile bool refreshTemp;
extern char diasem[][12];

#endif //GLOBALS_H
