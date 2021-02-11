#include "globals.h"
#include "restServer.hpp"

#define SLEEP_DELAY 3 // 60 //tiempo en segundos en modo deep-sleep
#define UPDATE_WEATHER_DATA 5 * 60 // tiempo entre actualizaciones datos tiempo
// #define UPDATE_WEATHER_DATA 5 * 60 // tiempo entre actualizaciones datos tiempo
static unsigned long ahora = 0, lastMillis = 0, startMillis; // tiempo;

bool pulsado = false;
bool modoSleep = false;
bool errorRTC = false;
bool restServer = false;
bool openweather_act = false;
// DEBUG true - activa puerto serie y desactiva display
bool DEBUG = true;

uint8_t pulsador = D4;
uint8_t DHT_pin = D6;
int BATT_A0;

void sleep();
void epaperMsg(const char *message );

void setup() {
    startMillis = millis(); //tiempo actual
    if(DEBUG) {
        Serial.begin(57600);
    } else  epaper_init();
    // Serial.println("inicializado puerto serie");
    // inicializar pantalla e-paper
   
    
    // Comprobamos si tenemos el RTC conectado
    if (!rtc.begin()) {
        epaperMsg("No hay un modulo RTC");
        errorRTC = true;
    }
    // inicializamos el sensor de temp/humedad
    // y damos tiempo a que este disponible para lectura
    dht.begin();
    delay(500); 
    // actualizamos los datos
    updateTime();
    updateTempHum();
    // leemos los datos del archivo json
    openweather_act = false;
    if(SPIFFSloadData()) {
        DateTime ahora = rtc.now();
        time_t segsAhora = ahora.unixtime();
        time_t diff = segsAhora - weather_data.dt;
        if(DEBUG) Serial.printf("dt: %ld  ahora: %ld  \ndiff(seg): %ld\n", weather_data.dt, segsAhora, diff);
        // comparamos la diferencia entre actualizaciones (en segundos)
        if ( diff > UPDATE_WEATHER_DATA ) {
            // datos de openweather
            if(!getOpenweatherData()) {
                if(DEBUG) Serial.println("no ha sido posible cargar los datos metereologicos");
            } else {
                // guardamos los datos en la memoria flash
                if(DEBUG) Serial.println("capturados y guardadados datos de openweather");
                SPIFFSsaveData();
                openweather_act = true;
            }     
        }
    }
    // mostramos los datos por pantalla
    epaperMostrarDatos();
    if(DEBUG) {
        int pje = getBatt();
        Serial.printf("bateria: %d - (%d)\n", BATT_A0, pje);
    } 
    lastMillis = millis(); //tiempo actual
}

void loop() {
    uint16_t tiempoMAX = 60000; //60 seg
    //if(errorRTC) return;

    ahora = millis();
    //comprobamos si hay que pasar a modo sleep en 1 seg
    if( !restServer && ahora - lastMillis > 1000 ) {
         if(DEBUG) Serial.printf("tiempo entrada en el sleep: %lu ms\n", ahora - startMillis);
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
        if(ahora - lastMillis < 5000) {
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
    if(DEBUG) {
        Serial.println("entrando en sleep");
    } else {
        epaperMostrarDatos();
    }
    delay(50);
    ESP.deepSleep(1e6 * SLEEP_DELAY); // duerme durante Sleep_delay segundos
}

// inicializa la conexión wifi con el router
int wifi_init() {
    int retries = 0;
    // asignacion de IP fija
    IPAddress staticIP(192,168,100,112);
    IPAddress gateway(192,168,100,100);
    IPAddress subnet(255,255,255,0);
    IPAddress DNS1(8,8,8,8);
    IPAddress DNS2(8,8,4,4);
    WiFi.mode(WIFI_STA);
    if(DEBUG) Serial.println("conectando a WiFi AP");
    WiFi.persistent(false); // para no regrabar los valores en la flash
    // WiFi.config(staticIP, gateway, subnet, DNS1, DNS2);
    // WiFi.begin(wifi_ssid, wifi_passwd);
    WiFi.begin();
    // WL_CONNECTED indica que se ha realizado la conexión con el AP
    while (WiFi.status() != WL_CONNECTED && (retries < MAX_WIFI_INIT_RETRY)) {
        retries++;
        delay(WIFI_RETRY_DELAY);
        if(DEBUG) Serial.print(".");
    }
    // Serial.printf("\nConectado a %s", wifi_ssid);
    if(WiFi.status() != WL_CONNECTED) {
        if(DEBUG) Serial.println("no ha sido posible conectar a la red wifi");
    } else {
        if(DEBUG) Serial.print("\nIP: ");
        if(DEBUG) Serial.println(WiFi.localIP());
        if(DEBUG) Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
    }
    return WiFi.status();
}

int getBatt() {
    // leemos la entrada analógica
    // y mapeamos el valor a un porcentaje
    BATT_A0 = analogRead(A0);
    // wemos d1 mini divisor 470K - 220K -> los valores se han tomado entre 7,4v y 8,4v
    // int valor = (int)map(BATT_A0, 435, 670, 0, 100);
   // para el ESP12E/F la entrada A0 tiene 1V max, por lo que el divisor será 1M - 100K
    // int valor = (int)map(BATT_A0, 720, 825, 0, 100); //valores para 2 celdas
    // alimentamos el circuito con una celda
    int valor = (int)map(BATT_A0, 330, 405, 0, 100);
    return valor; 
}