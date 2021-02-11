#include "globals.h"
#include "images/iconos.h"
#include "images/batt.h"

// mapping suggestion from Waveshare SPI e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V
// GxIO_Class io(SPI, /*CS=D8 SS*/ 1 /*TX*/, /*DC=D3*/ 0, /*RST=D4 2*/ 3 /*RX*/); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
// GxEPD_Class display(io, /*RST=D4 2*/ 3, /*BUSY=D2 4*/ 15 /*SS*/); // default selection of D4(=2), D2(=4)

//GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ 1, /*DC=4*/ 0, /*RST=2*/ 3, /*BUSY=5*/ 15));
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ 1, /*DC=4*/ 0, /*RST=2*/ 3, /*BUSY=5*/ 15));
#define DISABLE_DIAGNOSTIC_OUTPUT

void epaper_init() {
    display.init();
    display.init(115200, true, 2, false);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold12pt7b);
    display.setRotation(1);
}


unsigned char* getBattIcon() {
  int carga = getBatt();
  if( carga > 85 ) return bt100;
  if( carga > 70 ) return bt75;
  if( carga > 45 ) return bt50;
  if( carga > 20 ) return bt25;
  return bt00;
}

void epaperMostrarDatos() {
  uint8_t icono[648]; // iconos de 72x72
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  display.fillScreen(GxEPD_WHITE);
  uint8_t fw = 12, left_off = 14;
  display.setFont(&FreeSans12pt7b);
  display.drawXBitmap(left_off, 4, termometro_22x22, 22, 22, GxEPD_BLACK);
  display.setCursor(left_off + termometro_22x22_width , 22);
  display.printf("%2.2f", temp);
  display.drawXBitmap(left_off + fw * 6 + termometro_22x22_width + 5, 4, humedad_17x22, 17, 22, GxEPD_BLACK);
  display.setCursor(left_off + fw * 6 + termometro_22x22_width + humedad_17x22_width + 8 ,22);
  display.printf("%2.2f", hum);
  display.drawLine(0, 28, display.width(), 28, GxEPD_BLACK);
  // Hora
  fw = 24;
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(10, 45);
  display.print(diasem[dayOfWeek]);
  display.setCursor(10, 63);
  display.printf("%02d %s %02d", stFecha.dia, nommes[stFecha.mes-1], stFecha.anyo);
  display.setFont(&FreeSansBold24pt7b);
  display.setCursor(6, 110);
  display.printf("%02d:%02d", stHora.hora, stHora.min);
  display.drawLine(10 + fw * 5 + 6, 28, 10 + fw * 5 + 6, display.height(), GxEPD_BLACK);
 
  // representación datos metereológicos
  int fh = 9;
  display.setFont(&Org_01);
  display.setCursor(130 + 12, 28 + fh);
  display.printf("%s", weather_data.description.c_str());
  // si existe el icono lo representamos
  if( loadFileIcon(icono, weather_data.icon) ){
    display.drawXBitmap(140, 42, icono, 72, 72, GxEPD_BLACK);
  } 
  // display.drawRect(140, 42, 72, 72, GxEPD_BLACK);
  // datos openweather temp y humedad
  char str_valor[6];
  display.setFont(&FreeMonoBold9pt7b);
  display.drawXBitmap(276, 40, temp16, 16, 16, GxEPD_BLACK);
  display.setCursor(220, 52);
  dtostrf(weather_data.temp, 5, 2, str_valor);
  display.print(str_valor);
  display.drawXBitmap(276, 58, temp_min16, 16, 16, GxEPD_BLACK);
  display.setCursor(220, 70);
  // sprintf(str_valor,"%02.2f",weather_data.temp_min);
  dtostrf(weather_data.temp_min, 5, 2, str_valor);
  display.print(str_valor);
  // display.printf("%02.2f", weather_data.temp_min);
  display.drawXBitmap(276, 76, temp_max16, 16, 16, GxEPD_BLACK);
  display.setCursor(220, 88);
  // display.printf("%2.2f", weather_data.temp_max);
  dtostrf(weather_data.temp_max, 5, 2, str_valor);
  display.print(str_valor);
  display.drawXBitmap(278, 94,hum16, 16, 16, GxEPD_BLACK);
  display.setCursor(220, 106);
  // display.printf("%2.2f", weather_data.humidity);
  dtostrf(weather_data.humidity, 5, 2, str_valor);
  display.print(str_valor);

  display.setFont(&Org_01);
  if( openweather_act ) {
    display.setCursor(140, 114);
    display.print("actualizado");
  }
  display.setCursor(140, 123);
  display.printf("orto: %02d:%02d  ocaso: %02d:%02d",
    hour(weather_data.sunrise), minute(weather_data.sunrise),
    hour(weather_data.sunset), minute(weather_data.sunset));

  // modo sleep
  if( modoSleep ) {
    display.drawXBitmap(272, 4, sleep_22x22, 22, 22, GxEPD_BLACK);
    // display.hibernate();
  }
  // modo wifi on
  if( restServer ) {
    display.drawXBitmap(272, 4, wifi_22x22, 22, 22, GxEPD_BLACK);
    // display.setCursor(20,100);
    // display.print("WiFi On");
  }

  // icono de carga de bateria
  // display.setCursor(200,22);
  // display.setFont(&FreeSans9pt7b);
  // display.printf("%03d", getBatt());
  display.drawXBitmap(220, 5, getBattIcon(), 32, 18, GxEPD_BLACK);


  // while (display.nextPage());
  display.nextPage();
}
