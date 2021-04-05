#include "globals.h"
#include <ESP8266HTTPClient.h>
// #include openweather.h (PROGMEM) - iconos tiempo

#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50
const char* API_openweather = "http://api.openweathermap.org/data/2.5/weather?id=2512989&units=metric&lang=es&appid=";
const char* API_openweather_onecall = "http://api.openweathermap.org/data/2.5/onecall?lat=39.5694&lon=2.6502&units=metric&lang=es&exclude=minutely,hourly&appid=";
const char* jsonDataPath = "/weather/info.json";
const char* weatherkey = "/weather/weather.key";

WiFiClient client;
HTTPClient http;
DynamicJsonDocument w_doc(1400); 
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};
TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};
Timezone spainTZ(CEST, CET);

st_weather_data weather_data;

char urlfinal[255];
String key;

bool getOpenweatherData() { 
    if(WiFi.status() != WL_CONNECTED) {
       if(wifi_init() != WL_CONNECTED) {
           if(DEBUG) Serial.println("getOpenweatherData: sin acceso a wifi");
           return false;
       }
    }
    // añadimos la api key de openweather al path
    if(SPIFFS.begin()) {
        File fjson = SPIFFS.open(weatherkey, "r");
        key = fjson.readString();
        fjson.close();
        SPIFFS.end();
    }

    strcpy(urlfinal, API_openweather_onecall);
    strcat(urlfinal, key.c_str());
    if( !http.begin(client, urlfinal) ) { 
        if(DEBUG) Serial.printf("HTTP no ha sido posible conectar\n");
        return false;
    }
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.GET();
    if(DEBUG) Serial.printf("HTTP GET code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
        // preparamos el filtro con los datos que queremos obtener
        StaticJsonDocument<280> filtro;
        filtro["current"]["weather"][0]["description"] = true;
        filtro["current"]["weather"][0]["icon"] = true;
        filtro["current"]["temp"] = true;
        filtro["current"]["humidity"] = true;
        filtro["current"]["dt"] = true;
        filtro["daily"][0]["temp"]["min"] = true;
        filtro["daily"][0]["temp"]["max"] = true;
        filtro["daily"][0]["sunset"] = true;
        filtro["daily"][0]["sunrise"] = true;
        // capturamos los datos usando el filtro
        DeserializationError error = deserializeJson(w_doc, http.getStream(), DeserializationOption::Filter(filtro));
        if(error) {
            if(DEBUG) Serial.print(F("error en DeserializeJson (datos): "));
            if(DEBUG) Serial.println(error.f_str());
            return false;
        } 
        http.end();
        weather_data.description =  w_doc["current"]["weather"][0]["description"].as<String>();
        weather_data.icon =         w_doc["current"]["weather"][0]["icon"].as<String>();
        weather_data.dt =           spainTZ.toLocal(w_doc["current"]["dt"]);
        weather_data.temp =         w_doc["current"]["temp"];
        weather_data.humidity =     w_doc["current"]["humidity"];
        weather_data.temp_min =     w_doc["daily"][0]["temp"]["min"] ;
        weather_data.temp_max =     w_doc["daily"][0]["temp"]["max"] ;
        weather_data.sunset =       spainTZ.toLocal(w_doc["daily"][0]["sunset"]);
        weather_data.sunrise =      spainTZ.toLocal(w_doc["daily"][0]["sunrise"]);
        return true;
    } else {
        if(DEBUG) Serial.printf("[HTTP] error en GET: %s\n", http.errorToString(httpCode).c_str());
        http.end();
        return false;
    } 
}

// carga los datos desde la flash
bool SPIFFSloadData() {
    String data;
    if(DEBUG) Serial.println("SPIFFSloadData:");
    if(SPIFFS.begin()) {
        File fjson = SPIFFS.open(jsonDataPath, "r");
        data = fjson.readString();
        printf("file json: %s\n", data.c_str());
        fjson.close();
        SPIFFS.end();
        DeserializationError error = deserializeJson(w_doc, data);
        if(error) {
            if(DEBUG) Serial.print(F("error en DeserializeJson (datos): "));
            if(DEBUG) Serial.println(error.f_str());
            return false;
        } 
        weather_data.description =  w_doc["description"].as<String>();
        weather_data.icon =         w_doc["icon"].as<String>();
        weather_data.dt =           w_doc["dt"];
        weather_data.temp =         w_doc["temp"];
        weather_data.temp_min =     w_doc["temp_min"];
        weather_data.temp_max =     w_doc["temp_max"];
        weather_data.humidity =     w_doc["humidity"];
        weather_data.sunset =       w_doc["sunset"];
        weather_data.sunrise =      w_doc["sunrise"];
        if(DEBUG) Serial.printf("temp_min: %.2f  temp_max: %.2f\n", weather_data.temp_min, weather_data.temp_max);
        if(DEBUG) Serial.printf("sunset: %d  sunrise: %d\n", weather_data.sunset, weather_data.sunrise);
        return true;
    }
    return false;
}

// guarda los datos capturados a la memoria flash del esp8266
void SPIFFSsaveData() {
    DynamicJsonDocument jsonDatos(200);
    char buffer[200];

    jsonDatos["description"] = weather_data.description;
    jsonDatos["icon"]        = weather_data.icon;
    jsonDatos["temp"]        = weather_data.temp;
    jsonDatos["temp_min"]    = weather_data.temp_min;
    jsonDatos["temp_max"]    = weather_data.temp_max;
    jsonDatos["humidity"]    = weather_data.humidity;
    jsonDatos["dt"]          = weather_data.dt;
    jsonDatos["sunrise"]     = weather_data.sunrise;
    jsonDatos["sunset"]      = weather_data.sunset;

    serializeJson(jsonDatos, buffer);

    if(SPIFFS.begin()) {
        File json;
        if(SPIFFS.exists(jsonDataPath)) {
            json = SPIFFS.open(jsonDataPath, "r");
            if(DEBUG) Serial.printf("info.json: %s\n", json.readString().c_str());
            json.close();
        }
        json = SPIFFS.open(jsonDataPath, "w");
        // grabamos los nuevos datos
        json.println(buffer);
        json.close();
        SPIFFS.end();
    }
}

// función usada para crear los iconos en la memoria flash del esp8266
void saveFileIcon(int tam, uint8_t* icon, const char* fileName) {
      if(SPIFFS.begin()) {
        File ficon = SPIFFS.open(fileName, "w");
        //grabamos los bytes del icono
        uint8_t* buffer = new uint8_t[tam];
        memcpy_P(buffer, icon, tam);
        ficon.write(buffer, tam);
        ficon.close();
        if(DEBUG) Serial.printf("icono %s guardado\n", fileName);
        SPIFFS.end();
      }
}

// carga el icono en un array de bytes, si no existe el icono de noche se busca el de día
// devuelve true si la operación a podido completarse
bool loadFileIcon(uint8_t* icon, String fileName) {
    String path = "/weather/";
    path.concat(fileName);
    path.concat(".ico");

    // Serial.printf("icon path: %s\n", path.c_str());

    if(SPIFFS.begin()) {
        if(SPIFFS.exists(path)) {
            File ficon = SPIFFS.open(path, "r");
            // Serial.printf("cargados bytes: %d\n", ficon.size());
            ficon.read(icon, ficon.size());
            ficon.close();
            return true;
        } else {
            if(fileName.endsWith("n")) {
                String newfName = fileName.substring(0, 2);
                path = "/weather/";
                path.concat(newfName);
                path.concat("d.ico");
                if(DEBUG) Serial.printf("path modificado: %s\n", path.c_str());
                if(SPIFFS.exists(path)) {
                    File ficon = SPIFFS.open(path, "r");
                    ficon.read(icon, ficon.size());
                    ficon.close();
                    return true;
                }
            }
        }
        SPIFFS.end();
    }
    if(DEBUG) Serial.printf("el fichero %s no existe\n", fileName.c_str());
    return false;
}

    // relación de iconos guardados - #include openweather.h (PROGMEM)
    // saveFileIcon( 1300, i_01d, "/weather/01d.ico");
    // saveFileIcon( 1300, i_01n, "/weather/01n.ico");
    // saveFileIcon( 1300, i_02d, "/weather/02d.ico");
    // saveFileIcon( 1300, i_02n, "/weather/02n.ico");
    // saveFileIcon( 1300, i_03d, "/weather/03d.ico");
    // saveFileIcon( 1300, i_04d, "/weather/04d.ico");
    // saveFileIcon( 1300, i_09d, "/weather/09d.ico");
    // saveFileIcon( 1300, i_10d, "/weather/10d.ico");
    // saveFileIcon( 1300, i_10n, "/weather/10n.ico");
    // saveFileIcon( 1300, i_11d, "/weather/11d.ico");
    // saveFileIcon( 1300, i_13d, "/weather/13d.ico");
    // saveFileIcon( 1300, i_50d, "/weather/50d.ico");