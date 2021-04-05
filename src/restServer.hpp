#include <ESP8266mDNS.h>
#include "globals.h"
#include "index.html"

const char* wifi_ssid = "SSID";
const char* wifi_passwd = "PASSWORD";

#define HTTP_REST_PORT 80

ESP8266WebServer http_rest_server(HTTP_REST_PORT);
DynamicJsonDocument doc(200);
char msgBuffer[200];

void getHora() {
    doc.clear();
    updateTime();
    doc["hora"] = stHora.hora;
    doc["min"]  = stHora.min;
    doc["seg"]  = stHora.seg;
    serializeJson(doc, msgBuffer);
    http_rest_server.send(200, "application/json", msgBuffer);
}

void getFecha() {
    doc.clear();
    updateTime();
    doc["año"] = stFecha.anyo;
    doc["mes"] = stFecha.mes;
    doc["dia"] = stFecha.dia;
    serializeJson(doc, msgBuffer);
    http_rest_server.send(200, "application/json", msgBuffer);
}
void getTemp() {
    doc.clear();
    updateTempHum();
    doc["temp"] = temp;
    doc["humedad"] = hum;
    serializeJson(doc, msgBuffer);
    http_rest_server.send(200, "application/json", msgBuffer);
}

void getBattery() {
    doc.clear();
    int batt = getBatt();
    doc["batt"] = batt;
    doc["A0_input"]  = BATT_A0;
    serializeJson(doc, msgBuffer);
    http_rest_server.send(200, "application/json", msgBuffer);
}

void setHora() {
    int h, m, s;
    doc.clear();
    auto error = deserializeJson(doc, http_rest_server.arg("plain"));
    if(DEBUG) Serial.print("HTTP Method: ");
    if(DEBUG) Serial.println(http_rest_server.method());

    if( error ) {
        if(DEBUG) Serial.println("error interpretación json body");
        http_rest_server.send(400);
    }
    else {
        // se deben recibir obligatoriamente la hora y los minutos
        if( !doc.containsKey("hora") || !doc.containsKey("min") ) {
            if(DEBUG) Serial.println("deben recibirse la hora y los min (seg opcional)");
            http_rest_server.send(400);
        } else {
            h = doc["hora"];
            m = doc["min"];
            s = doc["seg"];
            if(DEBUG) Serial.printf("PUT recibido: %d:%d:%d\n", h, m, s);
            // cambiamos la hora
            grabarHora(h, m, s);
            http_rest_server.send(200);
        }
    }
}

void setFecha() {
    int d, m, a;
    doc.clear();
    auto error = deserializeJson(doc, http_rest_server.arg("plain"));
    if(DEBUG) Serial.print("HTTP Method: ");
    if(DEBUG) Serial.println(http_rest_server.method());

    if( error ) {
        if(DEBUG) Serial.println("error interpretación json body");
        http_rest_server.send(400);
    }
    else {
        // se deben recibir obligatoriamente la hora y los minutos
        if( !doc.containsKey("dia") || !doc.containsKey("mes") || !doc.containsKey("año") ) {
            if(DEBUG) Serial.println("deben recibirse dia, mes y año");
            http_rest_server.send(400);
        } else {
            //cambiamos la fecha por la recibida
            d = doc["dia"];
            m = doc["mes"];
            a = doc["año"];
            if(DEBUG) Serial.printf("PUT recibido: %d/%d/%d\n", d, m, a);
            // cambiamos la fecha
            grabarFecha(a, m, d);
            http_rest_server.send(200);
        }
    }
}

void inputProcess() {
    String dia="", mes="", anyo="", hora="", min="", seg="", ajuste="";
    String nomArg, valArg;
    for(int i=0; i < http_rest_server.args(); i++) {
        Serial.printf("%s: %s\n", http_rest_server.argName(i).c_str(), http_rest_server.arg(i).c_str());
        nomArg = http_rest_server.argName(i);
        valArg = http_rest_server.arg(i);
        dia = nomArg == "dia" ? valArg : dia;
        mes = nomArg == "mes" ? valArg : mes;
        anyo = nomArg == "anyo" ? valArg : anyo;
        hora = nomArg == "hora" ? valArg : hora;
        min = nomArg == "min" ? valArg : min;
        seg = nomArg == "seg" ? valArg : seg;
        ajuste = nomArg == "ajuste" ? valArg : ajuste;
    }
    if( ajuste == "fecha") {
        Serial.printf("dia: %s mes: %s año: %s\n", dia.c_str(), mes.c_str(), anyo.c_str());
        grabarFecha( int(anyo.c_str()), int(mes.c_str()), int(dia.c_str()) );
    } else {
        Serial.printf("hora: %s min: %s seg: %s\n", hora.c_str(), min.c_str(), seg.c_str());
        grabarHora( int(hora.c_str()), int(min.c_str()), int(seg.c_str()) );
    }
    http_rest_server.send(200, "text/html", "<html><body><h2>datos actualizados</h2></body></html>");
}


void server_routing() {
    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html", frontend);
    });
    http_rest_server.on("/frmFecha", HTTP_POST, inputProcess);

    // valores para rest
    http_rest_server.on("/hora",  HTTP_GET, getHora);
    http_rest_server.on("/hora",  HTTP_PUT, setHora);
    http_rest_server.on("/fecha", HTTP_GET, getFecha);
    http_rest_server.on("/fecha", HTTP_PUT, setFecha);
    http_rest_server.on("/temp",  HTTP_GET, getTemp);
    http_rest_server.on("/batt",  HTTP_GET, getBattery);
}

bool serverInit() {

    if (WiFi.status() != WL_CONNECTED) {
        wifi_init();
    }
    server_routing();

    http_rest_server.begin();
    return true;
}
