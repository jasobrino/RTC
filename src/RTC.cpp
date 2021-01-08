
#include "globals.h"

RTC_DS3231 rtc;
DHT dht(DHT_pin, DHT22);

st_hora stHora, stAlarm1;
st_fecha stFecha;

uint8_t pin_int;
uint8_t dayOfWeek;
float temp = 0, hum = 0;
float rtc_temp;
bool horaConsola = false;
 
volatile bool interrupt = false;
volatile bool alarm_on = true;
volatile bool refreshTemp = true;

char diasem[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};

// void mostrarHora() {
//     char str_hora[20];
//     sprintf(str_hora,"%02d/%02d/%04d %02d:%02d:%02d",
//             stFecha.dia, stFecha.mes, stFecha.anyo, stHora.hora, stHora.min, stHora.seg);
//     Serial.print(str_hora);
//     Serial.printf(" - %s - RTCTemp: %3.2fC", diasem[dayOfWeek], rtc_temp);
//     // getTempHum();
//     Serial.printf(" (temp: %3.3f hum: %3.3f\n", temp, hum);
//     // consultamos la alarma
//     if( rtc.alarmFired(1) ) {
//         Serial.println("Disparada alarma n1!!");
//         rtc.clearAlarm(1);
//     } 
// }  

void grabarHora(byte h, byte mi, byte s) {
    rtc.adjust(DateTime(stFecha.anyo, stFecha.mes, stFecha.dia, h, mi, s));
    updateTime();
}

void grabarFecha(int a, byte m, byte d) {
    rtc.adjust(DateTime(a, m, d,stHora.hora, stHora.min, stHora.seg));
    updateTime();
}

void updateTime() {
    DateTime now = rtc.now();
    stFecha.anyo = now.year();
    stFecha.mes  = now.month();
    stFecha.dia  = now.day();
    stHora.hora = now.hour();
    stHora.min  = now.minute();
    stHora.seg  = now.second();
    dayOfWeek = now.dayOfTheWeek();
}

void updateTempHum() {
    refreshTemp = false;
    temp = dht.readTemperature();
    hum  = dht.readHumidity();
}

void setUpdateTempHum() {
    refreshTemp = true;
}

void ICACHE_RAM_ATTR handleInterrupt() {
    interrupt = true;
}
