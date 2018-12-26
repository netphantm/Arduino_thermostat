//////// ESP8266/WeMos D1 Mini Pro - DS18B20 IoT Thermostat
//// Copyright 2018 © Hugo
//// https://github.com/netphantm/Arduino/tree/master/thermostat

//// include third party libraries
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SSD1306Wire.h"
#include <ArduinoJson.h>

//// initialize variables / hardware
#define ONE_WIRE_BUS 2  // DS18B20 pin D4 = GPIO2
#define RELAISPIN1 D1
#define RELAISPIN2 D2
#define PBSTR "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 80

const size_t bufferSize = JSON_OBJECT_SIZE(6) + 160;
const static String pFile = "/prefs.json";
unsigned long uptime = (millis() / 1000 );
unsigned long previousMillis = 0;
bool justFormatted = false;
bool emptyFile = false;
bool heater = false;
bool debug = true;
char lanIP[16];
String inetIP;
String str_c;
String str_last;
uint8_t sha1[20];
float temp_c;
float temp_last;
String webString;
String relaisState;
String SHA1;
String host;
String payload;
int httpsPort;
int interval;
float temp_min;
float temp_max;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
ESP8266WebServer server(80);
SSD1306Wire  display(0x3c, D6, D5);

//// read temperature from sensor / switch relay on or off
void getTemperature() {
  Serial.println("= getTemperature: ");
  str_last = str_c;
  // read temperature from the sensor
  uptime = (millis() / 1000 ); // Set uptime
  DS18B20.requestTemperatures();  // initialize temperature sensor
  temp_c = float(DS18B20.getTempCByIndex(0)); // read sensor
  temp_c = temp_c - 1; // calibrate your sensor, if needed
  str_c = String(temp_c, 2);
  delay(10);
  Serial.print(temp_c);
  display.setColor(BLACK);
  display.drawString(0, 50, str_last + "°C");
  display.setColor(WHITE);
  display.drawString(0, 50, str_c + "°C");
  display.display();
}

void switchRelais() {
  Serial.print("= switchRelais: ");

  if (heater) {
    if (temp_c <= temp_min) {
      digitalWrite(RELAISPIN1, HIGH);
      digitalWrite(RELAISPIN2, HIGH);
      Serial.println("Turn both Relais ON");
      relaisState = "ON";
      display.setColor(BLACK);
      display.drawString(0, 60, "Relais is OFF");
      display.setColor(WHITE);
      display.drawString(0, 60, "Relais is ON");
      display.display();
    } else if (temp_c >= temp_max) {
      digitalWrite(RELAISPIN1, LOW);
      digitalWrite(RELAISPIN2, LOW);
      Serial.println("Turn both Relais OFF");
      relaisState = "OFF";
      display.setColor(BLACK);
      display.drawString(0, 60, "Relais is ON");
      display.setColor(WHITE);
      display.drawString(0, 60, "Relais is OFF");
      display.display();
    }
  } else {
    if (temp_c >= temp_max) {
      digitalWrite(RELAISPIN1, HIGH);
      digitalWrite(RELAISPIN2, HIGH);
      Serial.println("Turn both Relais ON");
      relaisState = "ON";
      display.setColor(BLACK);
      display.drawString(0, 60, "Relais is OFF");
      display.setColor(WHITE);
      display.drawString(0, 60, "Relais is ON");
      display.display();
    } else if (temp_c <= temp_min) {
      digitalWrite(RELAISPIN1, LOW);
      digitalWrite(RELAISPIN2, LOW);
      Serial.println("Turn both Relais OFF");
      relaisState = "OFF";
      display.setColor(BLACK);
      display.drawString(0, 60, "Relais is ON");
      display.setColor(WHITE);
      display.drawString(0, 60, "Relais is OFF");
      display.display();
    }
    Serial.print(relaisState);
  }
  delay(10);
}

//// preferences file clear / read / write
void clearPrefsFile() {
  Serial.println("= clearPrefsFile");

  Serial.println("Please wait for SPIFFS to be formatted");
  SPIFFS.format();
  Serial.println("SPIFFS formatted");
  delay(10);
  // mark file as empty
  emptyFile = true;
  server.send(200, "text/plain", "HTTP CODE 200: OK, SPIFFS formatted, preferences cleared\n");
}

void readPrefsFile() {
  Serial.println("= readPrefsFile");

  File f = SPIFFS.open(pFile, "r");
  // open file for reading
  if (!f) {
    Serial.println(F("Preferences file read open failed"));
    emptyFile = true;
    return;
  }
  while (f.available()) {
    // deserialize JSON
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(f);
    if (!root.success()) {
      Serial.println(F("Error deserializing json, maybe you cleared the preferences file? Not updating logserver"));
      emptyFile = true;
      return;
    } else {
      emptyFile = false;
      SHA1 = root["SHA1"].as<String>();
      host = root["host"].as<String>(), sizeof(host);
      httpsPort = root["httpsPort"].as<int>(), sizeof(httpsPort);
      interval = root["interval"].as<long>(), sizeof(interval);
      temp_min = root["temp_min"].as<float>(), sizeof(temp_min);
      temp_max = root["temp_max"].as<float>(), sizeof(temp_max);
      heater = root["heater"].as<bool>(), sizeof(heater);
      debug = root["debug"].as<bool>(), sizeof(debug);
      Serial.println(F("Got Preferences from file"));
    }
  }
  f.close();
  delay(10);
}

void updatePrefsFile() {
  Serial.println("\n= updatePrefsFile");

  if (server.args() < 1 || server.args() > 8) {
    server.send(200, "text/plain", "HTTP CODE 400: Invalid Request\n");
    return;
  }
  Serial.println("Got new preferences");
  webString = "HTTP CODE 200: OK, Got new Preferences,\n";

  // get new settings from URL
  SHA1 = server.arg("SHA1");
  host = server.arg("host");
  httpsPort = server.arg("httpsPort").toInt();
  interval = server.arg("interval").toInt();
  temp_min = server.arg("temp_min").toInt();
  temp_max = server.arg("temp_max").toInt();
  heater = server.arg("heater").toInt();
  debug = server.arg("debug").toInt();

  // open file for writing
  File f = SPIFFS.open(pFile, "w");
  if (!f) {
    Serial.println(F("Failed to create file"));
    server.send(200, "text/plain", "HTTP CODE 200: OK, File write open failed, not updated\n");
    return;
  }
  // serialize JSON
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["SHA1"] = SHA1;
  root["host"] = host;
  root["httpsPort"] = httpsPort;
  root["interval"] = interval;
  root["temp_min"] = temp_min;
  root["temp_max"] = temp_max;
  root["heater"] = heater;
  root["debug"] = debug;

  // write JSON to file
  if (root.printTo(f) == 0) {
    Serial.println(F("Failed to write to file"));
    webString += "File write open failed\n";
  } else {
    Serial.println("Preferences file updated");
    webString += "Preferences file updated,\nJSON root: ";
    String output;
    root.printTo(output);
    webString += output;
    // mark file as not empty
    emptyFile = false;
  }
  f.close();
  server.send(200, "text/plain", webString);
}

//// local webserver handlers / send data to logserver
void handle_root() {
  server.send(200, "text/html", "<html><head></head><body><div align=\"center\"><h1>Nothing to see here! Move along...</h1></div></body></html>\n");
}

void handleNotFound(){
  server.send(404, "text/plain", "HTTP CODE 404: Not found\n");
}

void updateWebserver() {
  Serial.println("= updateWebserver");

  // configure path + query for sending to logserver
  if (emptyFile) {
    Serial.println(F("Error deserializing json, maybe you cleared the preferences file? Not updating logserver"));
    return;
  }
  String pathQuery = "/logtemp.php?&status=";
  pathQuery += relaisState;
  pathQuery += "&uptime=";
  pathQuery += uptime;
  pathQuery += "&temperature=";
  pathQuery += temp_c;
  pathQuery += "&IP=";
  pathQuery += lanIP;
  pathQuery += "&temp_min=";
  pathQuery += temp_min;
  pathQuery += "&temp_max=";
  pathQuery += temp_max;
  pathQuery += "&heater=";
  pathQuery += heater;
  pathQuery += "&debug=";
  pathQuery += debug;

  Serial.print(F("Connecting to https://"));
  Serial.print(host);
  Serial.println(pathQuery);

  BearSSL::WiFiClientSecure webClient;
  from_str();
  webClient.setFingerprint(sha1);
  HTTPClient https;
  //Serial.println("HTTPS begin...");
  if (https.begin(webClient, host, httpsPort, pathQuery)) {
    //Serial.println("HTTPS GET...");
    int httpCode = https.GET();
    if (httpCode > 0) {
      Serial.print(F("HTTPS GET OK, code: "));
      Serial.println(httpCode);
      if (httpCode == HTTP_CODE_OK) {
          String payload = https.getString();
          Serial.println(payload);
      }
    } else {
      Serial.print(F("HTTPS GET failed! Error: "));
      Serial.println(https.errorToString(httpCode).c_str());
    }
    //Serial.println("closing connection");
    https.end();
  } else {
    Serial.println(F("HTTPS Unable to connect"));
  }
}

//// Miscellaneous stuff
//// print variables for debug
void debug_vars() {
  // write debug to serial port
  Serial.println(F("- DEBUG -"));
  Serial.print(F("- LAN IP: "));
  Serial.println(lanIP);
  Serial.print(F("- Inet IP: "));
  Serial.println(inetIP);
  Serial.print(F("- uptime: "));
  Serial.println(uptime);
  Serial.print(F("- Temperature: "));
  Serial.println(temp_c);
  Serial.print(F("- RelaisState: "));
  Serial.println(relaisState);
  if (emptyFile) {
    Serial.print(F("- emptyFile: "));
    Serial.println(emptyFile);
    return;
  }
  Serial.print(F("- SHA1: "));
  Serial.println(SHA1);
  Serial.print(F("- host: "));
  Serial.println(host);
  Serial.print(F("- httpsPort: "));
  Serial.println(httpsPort);
  Serial.print(F("- interval: "));
  Serial.println(interval);
  Serial.print(F("- temp_min: "));
  Serial.println(temp_min);
  Serial.print(F("- temp_max: "));
  Serial.println(temp_max);
  Serial.print(F("- heater: "));
  Serial.println(heater);
  Serial.print(F("- debug: "));
  Serial.println(F("duh..."));
  Serial.println("");
}

//// transform SHA1 to binary
void from_str() {
  int j = 0;
  for (int i = 0; i < 60; i = i + 3) {
    String x = ("0x" + SHA1.substring(i, i+2));
    sha1[j] = strtoul(x.c_str(), NULL, 16);
    j++;
  }
}
 
//// print progress bar to console
void printProgress (unsigned long percentage) {
  long val = (unsigned long) (percentage + 1);
  unsigned long lpad = (unsigned long) (val * PBWIDTH /100);
  unsigned long rpad = PBWIDTH - lpad;
  printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
  fflush (stdout);
}

//// WiFi config mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println("Opening configuration portal");
  display.init();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 10, "# Entered config mode #");
  display.drawString(0, 30, "ID/pass: NDND/pass4esp");
  display.drawString(0, 40, "Config IP: 10.0.1.1");
  display.display();
}

//// setup / first run
void setup(void) {
  Serial.begin(115200); // Start Serial 
 
  WiFiManager wifiManager;
  wifiManager.setTimeout(300);
  wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect("Clamps","pass4esp")) {
    delay(5000);
    Serial.println(F("Failed to connect and hit timeout, restarting..."));
    ESP.reset();
  }
  Serial.print(F("Seconds in void setup() WiFi connection: "));
  int connRes = WiFi.waitForConnectResult();
  Serial.println(connRes);
  if (WiFi.status()!=WL_CONNECTED) {
      Serial.println(F("Failed to connect to WiFi, resetting in 5 seconds"));
      delay(5000);
      ESP.reset();
  } else {
    if (!MDNS.begin("esp8266"))   { Serial.println(F("Error setting up mDNS responder")); }
    else                          { Serial.println(F("mDNS responder started")); }
  }
  String WiFi_Name = WiFi.SSID();
  IPAddress ip = WiFi.localIP();
  sprintf(lanIP, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  Serial.print("LAN IP: ");
  Serial.println(lanIP);

  WiFiClient client;
  HTTPClient http;
  //Serial.println(F("HTTP begin..."));
  if (http.begin(client, "http://ipinfo.io/ip")) { // GET Inet IP
    http.addHeader("Content-Type", "application/json");
    //Serial.println(F("HTTPS GET..."));
    int httpCode = http.GET();
    if(httpCode > 0) {
      Serial.print(F("HTTP response code "));
      Serial.println(httpCode);
      inetIP = http.getString();
      Serial.println("Inet IP: " + inetIP);
      display.init();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0,  0, "Connected to WiFi");
      display.drawString(0, 10, "WiFi AP SSID: " + WiFi_Name);
      display.drawString(0, 20, "LAN IP: " + String("") + lanIP);
      display.drawString(0, 30, "Inet IP: " + inetIP);
      display.display();
    } else {
      Serial.print(F("HTTPS GET failed! Error: "));
      Serial.println(http.errorToString(httpCode).c_str());
    }
    http.end();
    delay(50);
  } else {
    Serial.println(F("[HTTPS] Unable to connect"));
  }

  SPIFFS.begin(); // initialize SPIFFS
  /*
  Serial.println("Please wait for SPIFFS to be formatted");
  SPIFFS.format(); // remove comment block to clear SPIFFS data on boot
  justFormatted = true;
  */

  File f = SPIFFS.open(pFile, "r");
  if (!f) {
    // no data file, format SPIFFS
    if(! justFormatted) {
      Serial.println(F("Please wait for SPIFFS to be formatted"));
      SPIFFS.format();
    }
    Serial.println(F("SPIFFS formatted"));
  }
  f.close();

  pinMode(RELAISPIN1, OUTPUT);
  pinMode(RELAISPIN2, OUTPUT);
  // start with relais OFF
  digitalWrite(RELAISPIN1, LOW);
  digitalWrite(RELAISPIN2, LOW);
  relaisState = "OFF";
 
  // turn on if needed
  readPrefsFile();
  if (!emptyFile) {
    getTemperature();
    switchRelais();
    if (debug)
      debug_vars();
  }

  // web client handlers
  server.onNotFound(handleNotFound);
  server.on("/", handle_root);
  server.on("/update", []() {
    updatePrefsFile();
    getTemperature();
    switchRelais();
    if (debug)
      debug_vars();
  });
  server.on("/clear", []() {
    clearPrefsFile();
  });

  server.begin();
  Serial.println(F("HTTP Server started. Giving you 5 seconds to send data..."));
  delay(5000);
  Serial.println(F("Starting loop..."));
  Serial.println("");
} 
 
// main loop
void loop(void) {
  // start main loop
  unsigned long currentMillis = millis();
  unsigned long past = currentMillis - previousMillis;
  if (past > interval) {
    Serial.println(F(" Interval passed"));
    // save the last time sensor was read
    previousMillis = currentMillis;

    if (emptyFile) {
      Serial.println(F("Switching relais off, as no temp_min/temp_max was set"));
      digitalWrite(RELAISPIN1, LOW);
      digitalWrite(RELAISPIN2, LOW);
      Serial.println(F("Waiting for settings to be sent..."));
    } else {
      readPrefsFile();
      getTemperature();
      if (debug)
        debug_vars();
      switchRelais();
      updateWebserver();
    }

    // Failsafe interval
    if (interval < 6) {
      interval = 20;
    }
  } else {
    delay(100);
    printProgress(past * 100 / interval);
  }
  server.handleClient();
}
