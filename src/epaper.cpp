#include "globals.h"
#include "images/termometro_22x22.h"
#include "images/humedad_17x22.h"
#include "images/sleep_22x22.h"
#include "images/wifi_22x22.h"

// mapping suggestion from Waveshare SPI e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V
// GxIO_Class io(SPI, /*CS=D8 SS*/ 1 /*TX*/, /*DC=D3*/ 0, /*RST=D4 2*/ 3 /*RX*/); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
// GxEPD_Class display(io, /*RST=D4 2*/ 3, /*BUSY=D2 4*/ 15 /*SS*/); // default selection of D4(=2), D2(=4)
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ 1, /*DC=4*/ 0, /*RST=2*/ 3, /*BUSY=5*/ 15));

void epaper_init() {
    display.init(115200);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold12pt7b);
    display.setRotation(1);
}

void epaperMostrarDatos() {
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
  // display.drawRoundRect(left_off - 4, 1, left_off + fw * 9 + 4, 22, 3, GxEPD_BLACK);
  // display.drawRoundRect(left_off + fw * 11 - 4, 1, left_off + fw * 8 + 4, 22, 3, GxEPD_BLACK);
  // Hora
  display.setFont(&FreeSansBold24pt7b);
  fw = 24;
  display.setCursor(20, 75);
  display.printf("%02d:%02d", stHora.hora, stHora.min);
  display.setFont(&FreeSans9pt7b);
  display.setCursor(20 + fw * 5 + 15, 50);
  display.print(diasem[dayOfWeek]);
  display.setCursor(20 + fw * 5 + 15, 75);
  display.setFont(&FreeSans12pt7b);
  display.printf("%02d/%02d/%02d", stFecha.dia, stFecha.mes, stFecha.anyo);
  display.drawLine(20 + fw * 5 + 6, 28, 20 + fw * 5 + 6, 75, GxEPD_BLACK);
  // modo sleep
  if( modoSleep ) {
    display.drawXBitmap(272, 4, sleep_22x22, 22, 22, GxEPD_BLACK);
    // display.hibernate();
  }
  if( restServer ) {
    display.drawXBitmap(272, 4, wifi_22x22, 22, 22, GxEPD_BLACK);
    // display.setCursor(20,100);
    // display.print("WiFi On");
  }
  // display.setCursor(10,32);
  // display.setFont(&FreeMonoBold9pt7b);

  // while (display.nextPage());
  display.nextPage();
}