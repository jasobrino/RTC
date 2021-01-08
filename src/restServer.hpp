#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "globals.h"

const char* wifi_ssid = "mongerMax";
const char* wifi_passwd = "elprimodemartos";

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

ESP8266WebServer http_rest_server(HTTP_REST_PORT);

void getHora() {
    DynamicJsonDocument doc(200);
    char msgBuffer[200];
    updateTime();
    doc["hora"] = stHora.hora;
    doc["min"]  = stHora.min;
    doc["seg"]  = stHora.seg;
    serializeJson(doc, msgBuffer);
    http_rest_server.send(200, "application/json", msgBuffer);
}

void getFecha() {
    DynamicJsonDocument doc(200);
    char msgBuffer[200];
    updateTime();
    doc["año"] = stFecha.anyo;
    doc["mes"] = stFecha.mes;
    doc["dia"] = stFecha.dia;
    serializeJson(doc, msgBuffer);
    http_rest_server.send(200, "application/json", msgBuffer);
}
void getTemp() {
    DynamicJsonDocument doc(200);
    char msgBuffer[200];
    updateTempHum();
    doc["temp"] = temp;
    doc["humedad"] = hum;
    serializeJson(doc, msgBuffer);
    http_rest_server.send(200, "application/json", msgBuffer);
}

void setHora() {
    DynamicJsonDocument doc(200);
    int h, m, s;

    auto error = deserializeJson(doc, http_rest_server.arg("plain"));
    Serial.print("HTTP Method: ");
    Serial.println(http_rest_server.method());

    if( error ) {
        Serial.println("error interpretación json body");
        http_rest_server.send(400);
    }
    else {
        // se deben recibir obligatoriamente la hora y los minutos
        if( !doc.containsKey("hora") || !doc.containsKey("min") ) {
            Serial.println("deben recibirse la hora y los min (seg opcional)");
            http_rest_server.send(400);
        } else {
            h = doc["hora"];
            m = doc["min"];
            s = doc["seg"];
            Serial.printf("PUT recibido: %d:%d:%d\n", h, m, s);
            // cambiamos la hora
            grabarHora(h, m, s);
            http_rest_server.send(200);
        }
    }
}

void setFecha() {
    DynamicJsonDocument doc(200);
    int d, m, a;

    auto error = deserializeJson(doc, http_rest_server.arg("plain"));
    Serial.print("HTTP Method: ");
    Serial.println(http_rest_server.method());

    if( error ) {
        Serial.println("error interpretación json body");
        http_rest_server.send(400);
    }
    else {
        // se deben recibir obligatoriamente la hora y los minutos
        if( !doc.containsKey("dia") || !doc.containsKey("mes") || !doc.containsKey("año") ) {
            Serial.println("deben recibirse dia, mes y año");
            http_rest_server.send(400);
        } else {
            //cambiamos la fecha por la recibida
            d = doc["dia"];
            m = doc["mes"];
            a = doc["año"];
            Serial.printf("PUT recibido: %d/%d/%d\n", d, m, a);
            // cambiamos la fecha
            grabarFecha(a, m, d);
            http_rest_server.send(200);
        }
    }
}
void server_routing() {
    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html",
            "ESP8266 REST Web Server activo");
    });
    http_rest_server.on("/hora",  HTTP_GET, getHora);
    http_rest_server.on("/hora",  HTTP_PUT, setHora);
    http_rest_server.on("/fecha", HTTP_GET, getFecha);
    http_rest_server.on("/fecha", HTTP_PUT, setFecha);
    http_rest_server.on("/temp",  HTTP_GET, getTemp);

}


// inicializa la conexión wifi con el router
int init_wifi() {
    int retries = 0;

    // asignacion de IP fija
    IPAddress staticIP(192,168,100,112);
    IPAddress gateway(192,168,100,100);
    IPAddress subnet(255,255,255,0);

    Serial.println("conectando a WiFi AP");
    // u8x8.println("iniciando server");
    WiFi.persistent(false); // para no regrabar los valores en la flash
    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP, 1);
    // WiFi.begin(wifi_ssid, wifi_passwd);
    WiFi.begin();
    WiFi.config(staticIP, gateway, subnet);
    // WL_CONNECTED indica que se ha realizado la conexión con el AP
    while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
        retries++;
        delay(WIFI_RETRY_DELAY);
        Serial.print(".");
    }
    return WiFi.status(); 
}

bool serverInit() {

    if (init_wifi() == WL_CONNECTED) {
        Serial.printf("Conectado a %s", wifi_ssid);
        // Serial.print(wifi_ssid);
        Serial.print(" --- IP: ");
        Serial.println(WiFi.localIP());
        Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
        delay(200);
        //iniciar mDNS como rtc.local
        if(!MDNS.begin("wemosrtc")) {
            Serial.println("error iniciando mDNS");
        }
    }
    else {
        Serial.print("Error conectando a ");
        Serial.println(wifi_ssid);
        return false;
    }

    server_routing();

    http_rest_server.begin();
    return true;
}
