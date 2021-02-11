
#ifndef GLOBALS_H
#define GLOBALS_H

// #include <Arduino.h>
#include <Ticker.h>
#include <RTClib.h>
#include <DHT.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/Picopixel.h>
#include <Fonts/TomThumb.h>
#include <Fonts/Org_01.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include <Timezone.h> // ajustes para la zona horaria
#include <FS.h>

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
struct st_weather_data {
    String description;
    String icon;
    float  temp;
    float  temp_min;
    float  temp_max;
    float  humidity;
    time_t dt;
    time_t sunrise;
    time_t sunset;
    int    cod;
};

// cabecera de funciones
void ICACHE_RAM_ATTR handleInterrupt();
// funciones en RTC.cpp
void updateTempHum();
void updateTime();
void grabarHora(byte h, byte mi, byte s);
void grabarFecha(int a, byte m, byte d);
void setUpdateTempHum();
void updateTempHum();
// funciones en main
int wifi_init();
int getBatt();

//funciones en oled.hpp
// void OledInit();
// void oledVerHora(st_hora h);
// void oledVerTemp(float t, float h);
// extern U8X8_SH1106_128X64_NONAME_HW_I2C u8x8;

// funciones en epaper.cpp
void epaper_init();
void epaperMostrarDatos();

// funciones en openweather.cpp
bool SPIFFSloadData();
void SPIFFSsaveData();
bool getOpenweatherData();
void saveFileIcon(int tam, uint8_t* icon, const char* fileName);
bool loadFileIcon(uint8_t* icon, String fileName);

#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

// variables globales 
extern const char* wifi_ssid;
extern const char* wifi_passwd;
extern st_weather_data weather_data;

extern uint8_t pulsador;
extern uint8_t DHT_pin;
extern int BATT_A0;
extern bool modoSleep;
extern bool restServer;
extern bool openweather_act;
extern bool DEBUG;

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
extern char diasem[7][12];
extern char nommes[12][4];

#endif //GLOBALS_H
