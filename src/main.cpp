
#include "globals.h"
#include "restServer.hpp"

static unsigned long ahora = 0, lastMillis = 0, tiempo;

bool pulsado = false;
bool modoSleep = false;
bool errorRTC = false;
bool restServer = false;

uint8_t pulsador = D4;
uint8_t DHT_pin = D6;

void sleep();
void epaperMsg(const char *message );

void setup() {

    // inicializar pantalla e-paper
    epaper_init();
    // Comprobamos si tenemos el RTC conectado
    if (!rtc.begin()) {
        epaperMsg("No hay un modulo RTC");
        errorRTC = true;
    }
    if(!errorRTC) {
        // inicializamos el sensor de temp/humedad
        // y damos tiempo a que este disponible para lectura
        dht.begin();
        delay(500); 
        // actualizamos los datos y mostramos la información
        updateTime();
        updateTempHum();
        epaperMostrarDatos();
        lastMillis = millis(); //tiempo actual
    }
}

void loop() {
    uint16_t tiempoMAX = 60000; //60 seg
    if(errorRTC) return;

    ahora = millis();
    //comprobamos si hay que pasar a modo sleep en 2 seg
    if( !restServer && ahora - lastMillis > 2000 ) {
        sleep();
    }
    if( ahora - lastMillis > tiempoMAX ) {
        // máximo tiempo que permanecerá despierto antes de dormir
        sleep();
    }
    // lectura pulsador
    uint8_t estadoPulsador = digitalRead(pulsador);
    if( estadoPulsador == LOW ) { //pulsado
        //contamos el tiempo que se mantiene pulsado
        delay(50); // eliminar rebotes
        lastMillis = millis();
        //esperamos que se suelte el pulsador
        while( digitalRead(pulsador) == LOW ); 
        if(restServer) {
            //si el servidor ya estaba activo entonces entramos en modo sleep
            sleep();
        }
        ahora = millis();
        if(ahora - lastMillis < 2000) {
            lastMillis = ahora;
            //quitamos el ahorro de energia
            modoSleep = false;
            // arrancamos el restServer
            if( serverInit() ) {
                restServer = true;
                epaperMostrarDatos();
            }
        }

    }
    // escuchamos las peticiones REST
    if( restServer ) {
          http_rest_server.handleClient();
    }
}


void epaperMsg(const char *message ) {
  display.firstPage();
  display.setFont(&FreeSans9pt7b);
  display.println();
  display.println( message );
  display.nextPage();
  delay(300);
}

// pasamos a modo sleep durante 60 seg.
void sleep() {
    // pulsado = true;
    modoSleep = true;
    restServer = false;
    epaperMostrarDatos();
    delay(50);
    ESP.deepSleep(1e6 * 60);
}
