
#include "globals.h"
#include "restServer.hpp"
// #include "oled.hpp"

Ticker timerReloj, timerTemp;
static unsigned long ahora = 0, lastMillis = 0;
// uint8_t pulsador = D4;
bool pulsado = false;
bool modoSleep = false;

uint8_t pulsador = D4;
uint8_t DHT_pin = D6;
void sleep();

void start() {
    lastMillis = millis();
    // WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
    // WiFi.forceSleepBegin();
    // Serial.flush();
    Serial.begin(115200);
    // inicializamos la pantalla oled
    // oledInit();
    // u8x8.clearDisplay();
    // inicializar pantalla e-paper
    epaper_init();
    // Comprobamos si tenemos el RTC conectado
    if (!rtc.begin()) {
        Serial.println("No hay un módulo RTC");
    } else {
        Serial.println("\nmódulo RTC conectado");
        // u8x8.println("RTC conectado");
        // mostrarHora();
    }
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // inicializamos el sensor de termp/humedad
    dht.begin();
    // delay(1500);

    // mostrarHora();
    
    delay(1000);
    updateTempHum();
    updateTime();
    // mostrarHora();
   
    epaperMostrarDatos();

    // display.update();
    

    // borramos las alarmas
    rtc.clearAlarm(1);
    rtc.disableAlarm(1);

    //actualizamos el reloj con un timer
    // timerReloj.attach(3.0, updateTime);
    //actualización de los valores de temp y humedad
    timerTemp.attach(2.0, setUpdateTempHum);

    //borramos la pantalla
    // u8x8.clear();
    // arrancamos el restServer
    if( serverInit() ) {
        Serial.println("HTTP REST Server iniciado");
    }

}


void setup() {

    start();

    //poner interrupcion
    // pinMode(pin_int, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(pin_int), handleInterrupt, FALLING);
}



void loop() {

    uint8_t byte = 0;
    char buf_data[13];

    ahora = millis();
    // Serial.printf("tiempo: %l ms\n", (ahora - lastMillis));
    lastMillis = ahora;
    // WiFi.forceSleepWake();
    // WiFi.setSleepMode(WIFI_NONE_SLEEP);
    // while( millis() - ahora < 5000) {
    http_rest_server.handleClient();
    // }    
    // delay(100);
    // WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
    // WiFi.forceSleepBegin(10e3 * 100);
    // WiFi.forceSleepBegin();


    // delay(15000); //esperamos unos segundos
    // ESP.deepSleep(20e6);  

    //comprobamos si hay que leer la temp y humedad
    if( refreshTemp ) {
        updateTempHum();
        // Serial.println("temperatura actualizada");
    }

    //   uint32_t sleep_time_in_ms = 1000;
    //   wifi_set_opmode(NULL_MODE);
    //   wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
    //   wifi_fpm_open();
    // //   wifi_fpm_set_wakeup_cb(callback);
    //   wifi_fpm_do_sleep(sleep_time_in_ms );
    //   delay(sleep_time_in_ms + 1);
  

    //ver si se ha disparado la interrupción
    // if( interrupt ) {
    //     interrupt = false;
    //     Serial.printf("pulso: %06d\n", pulsos);
    // }

    // lectura pulsador
    uint8_t estadoPulsador = digitalRead(pulsador);
    if( !pulsado ) {   
        if ( estadoPulsador == LOW ) {
            sleep();
        }
    } else if( estadoPulsador == HIGH ) {
        pulsado = false;
        delay(50);
        Serial.println("pulsador off");
    }


    //comprobamos si se recibe alguna instrucción por el puerto serie
    if(Serial.available() > 0) {
        byte = Serial.read();
        switch(byte) {
            case 'm': horaConsola = true;
                      break;
            case 'p': horaConsola = false;
                      //tempo.detach();
                      break;
            case 'a':
                Serial.readBytes(buf_data, 6);
                stAlarm1.seg  = buf_data[5]-'0' + (buf_data[4]-'0') * 10;
                stAlarm1.min  = buf_data[3]-'0' + (buf_data[2]-'0') * 10;
                stAlarm1.hora = buf_data[1]-'0' + (buf_data[0]-'0') * 10;
                Serial.printf("alarma activa a las %02d:%02d:%02d\n", stAlarm1.hora, stAlarm1.min, stAlarm1.seg);
                rtc.clearAlarm(1);
                rtc.disableAlarm(1);
                rtc.setAlarm1(DateTime(0,0,0,stAlarm1.hora, stAlarm1.min, stAlarm1.seg), DS3231_A1_Minute);
                break;
            case 's': 
                Serial.readBytes(buf_data,12);
                Serial.printf("hora recibida: %s\n", buf_data);
                stHora.hora  = buf_data[7]-'0' + (buf_data[6]-'0') * 10;
                stHora.min   = buf_data[9]-'0' + (buf_data[8]-'0') * 10;
                stHora.seg   = buf_data[11]-'0' + (buf_data[10]-'0') * 10;
                stFecha.dia  = buf_data[5]-'0' + (buf_data[4]-'0') * 10;
                stFecha.mes  = buf_data[3]-'0' + (buf_data[2]-'0') * 10;
                stFecha.anyo = (buf_data[1]-'0' + (buf_data[0]-'0') * 10) + 2000;
                grabarFecha(stFecha.anyo, stFecha.mes, stFecha.dia);
                grabarHora(stHora.hora, stHora.min, stHora.seg);
                break;
            case 't': 
                Serial.printf("Temp: %.2f - Humedad: %.2f\n", temp, hum);
                break;
            default: Serial.printf("caracter no reconocido: %c\n", byte);
        }
    }

    delay(100);
}

void sleep() {
    pulsado = true;
    Serial.println("pulsador on");
    modoSleep = true;
    epaperMostrarDatos();
    // display.update();
    delay(50);
    ESP.deepSleep(1e6 * 10);
}
