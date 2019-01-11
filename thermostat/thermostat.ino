//// ESP8266/WeMos D1 Mini Pro - DS18B20 IoT Thermostat
//// Copyright 2018 © Hugo
//// https://github.com/netphantm/Arduino

//// include third party libraries
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

//// initialize variables / hardware
#define ONE_WIRE_BUS D3
#define RELAISPIN1 D1
#define RELAISPIN2 D2
#define TOUCHPIN1 D7
#define TOUCHPIN2 D8
#define PBSTR "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 79

const size_t bufferSize = JSON_OBJECT_SIZE(6) + 160;
const static String pFile = "/settings.txt";
unsigned long uptime = (millis() / 1000 );
unsigned long previousMillis = 0;
bool emptyFile = false;
bool heater = false;
bool manual;
bool debug;
char lanIP[16];
String inetIP;
String str_c;
String mode;
String WiFi_Name;
uint8_t sha1[20];
float temp_c;
String webString;
String relaisState;
String SHA1;
String loghost;
String epochTime;
String hostname = "Joey";
uint16_t color;
int httpsPort;
int interval;
float temp_min;
float temp_max;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
ESP8266WebServer server(80);
TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

//// read temperature from sensor / switch relay on or off
void getTemperature() {
  Serial.print("= getTemperature: ");
  String str_last = str_c;
  // read temperature from the sensor
  uptime = (millis() / 1000 ); // Refresh uptime
  DS18B20.requestTemperatures();  // initialize temperature sensor
  temp_c = float(DS18B20.getTempCByIndex(0)); // read sensor
  yield();
  temp_c = temp_c - 6; // calibrate your sensor, if needed
  delay(10);
  Serial.println(temp_c);
}

void switchRelais() {
  Serial.println("= switchRelais: ");
  if (manual) {
    return;
  } else {
    if (heater) {
      if (temp_c <= temp_min) {
        Serial.println("Auto turned relais ON");
        toggleRelais(1);
      } else if (temp_c >= temp_max) {
        Serial.println("Auto turned relais OFF");
        toggleRelais(0);
      }
    } else {
      if (temp_c >= temp_max) {
        Serial.println("Auto turned relais ON");
        toggleRelais(1);
      } else if (temp_c <= temp_min) {
        Serial.println("Auto turned relais OFF");
        toggleRelais(0);
      }
    }
  }
}

void toggleRelais(bool sw) {
  if (sw) {
    relaisState = "ON";
  } else {
    relaisState = "OFF";
  }
  digitalWrite(RELAISPIN1, sw);
  digitalWrite(RELAISPIN2, sw);
  yield();
}

//// settings file clear / read / write
void clearSettingsFile() {
  Serial.println("= clearSettingsFile");

  Serial.println("Please wait for SPIFFS to be formatted");
  SPIFFS.format();
  Serial.println("SPIFFS formatted");
  yield();
  // mark file as empty
  emptyFile = true;
  server.send(200, "text/plain", "HTTP CODE 200: OK, SPIFFS formatted, settings cleared\n");
}

void readSettingsFile() {
  Serial.println("= readSettingsFile");

  File f = SPIFFS.open(pFile, "r");
  // open file for reading
  if (!f) {
    Serial.println(F("Settings file read open failed"));
    emptyFile = true;
    return;
  }
  while (f.available()) {
    // deserialize JSON
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(f);
    if (!root.success()) {
      Serial.println(F("Error deserializing json, maybe you cleared the settings file? Not updating logserver"));
      emptyFile = true;
      return;
    } else {
      emptyFile = false;
      SHA1 = root["SHA1"].as<String>();
      loghost = root["loghost"].as<String>(), sizeof(loghost);
      httpsPort = root["httpsPort"].as<int>(), sizeof(httpsPort);
      interval = root["interval"].as<long>(), sizeof(interval);
      temp_min = root["temp_min"].as<float>(), sizeof(temp_min);
      temp_max = root["temp_max"].as<float>(), sizeof(temp_max);
      heater = root["heater"].as<bool>(), sizeof(heater);
      manual = root["manual"].as<bool>(), sizeof(manual); // this overrides manual mode setting from the settings file
      debug = root["debug"].as<bool>(), sizeof(debug);
      Serial.println(F("Got Settings from file"));
    }
  }
  f.close();
}

void updateSettings() {
  Serial.println("= updateSettings");

  if (server.args() < 1 || server.args() > 9) {
    server.send(200, "text/plain", "HTTP CODE 400: Invalid Request\n");
    return;
  }
  Serial.println("Got new settings");
  webString = "HTTP CODE 200: OK, Got new settings,\n";

  // get new settings from URL
  SHA1 = server.arg("SHA1");
  loghost = server.arg("loghost");
  httpsPort = server.arg("httpsPort").toInt();
  interval = server.arg("interval").toInt();
  epochTime = server.arg("epochTime").toInt();
  uptime = server.arg("uptime").toInt();
  temp_c = server.arg("temp_c").toInt();
  relaisState = server.arg("relaisState");
  temp_min = server.arg("temp_min").toInt();
  temp_max = server.arg("temp_max").toInt();
  heater = server.arg("heater").toInt() | 0;
  manual = server.arg("manual").toInt() | 0;
  debug = server.arg("debug").toInt() | 0;

  writeSettingsFile();
  server.send(200, "text/plain", webString);
}

void writeSettingsFile() {
  Serial.println("= writeSettingsFile");

  // open file for writing
  File f = SPIFFS.open(pFile, "w");
  if (!f) {
    Serial.println(F("Failed to create settings file"));
    server.send(200, "text/plain", "HTTP CODE 200: OK, File write open failed! settings not saved\n");
    return;
  }
  // serialize JSON
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["SHA1"] = SHA1;
  root["loghost"] = loghost;
  root["httpsPort"] = httpsPort;
  root["interval"] = interval;
  root["temp_min"] = temp_min;
  root["temp_max"] = temp_max;
  root["heater"] = heater;
  root["manual"] = manual;
  root["debug"] = debug;

  // write JSON to file
  if (root.printTo(f) == 0) {
    Serial.println(F("Failed to write to file"));
    webString += "File write open failed\n";
  } else {
    Serial.println("Settings file updated");
    webString += "Settings file updated,\nJSON root: ";
    String output;
    root.printTo(output);
    webString += output;
    // mark file as not empty
    emptyFile = false;
  }
  f.close();
  if (digitalRead(RELAISPIN1) == 1 && digitalRead(RELAISPIN2) == 1) {
    relaisState = "ON";
  } else {
    relaisState = "OFF";
  }
}

//// local webserver handlers / send data to logserver
void handleRoot() {
  server.send(200, "text/html", "<html><head></head><body><div align=\"center\"><h1>Nothing to see here! Move along...</h1></div></body></html>\n");
}

void handleNotFound(){
  server.send(404, "text/plain", "HTTP CODE 404: Not found\n");
}

void updateWebserver() {
  Serial.println("= updateWebserver");

  // configure path + query for sending to logserver
  if (emptyFile) {
    Serial.println(F("Empty settings file, maybe you cleared it? Not updating logserver"));
    return;
  }
  String pathQuery = "/logtemp.php?&status=";
  pathQuery += relaisState;
  pathQuery += "&temperature=";
  pathQuery += temp_c;
  pathQuery += "&hostname=";
  pathQuery += hostname;
  pathQuery += "&temp_min=";
  pathQuery += temp_min;
  pathQuery += "&temp_max=";
  pathQuery += temp_max;
  pathQuery += "&heater=";
  pathQuery += heater;
  pathQuery += "&manual=";
  pathQuery += manual;
  pathQuery += "&debug=";
  pathQuery += debug;

  Serial.print(F("Connecting to https://"));
  Serial.print(loghost);
  Serial.println(pathQuery);

  BearSSL::WiFiClientSecure webClient;
  from_str();
  webClient.setFingerprint(sha1);
  HTTPClient https;
  if (https.begin(webClient, loghost, httpsPort, pathQuery)) {
    int httpCode = https.GET();
    if (httpCode > 0) {
      Serial.print(F("HTTPS GET OK, code: "));
      Serial.println(httpCode);
      if (httpCode == HTTP_CODE_OK) {
          epochTime = https.getString();
          Serial.print("Time on log update: ");
          Serial.println(epochTime);
      }
    } else {
      Serial.print(F("HTTPS GET failed! Error: "));
      Serial.println(https.errorToString(httpCode).c_str());
      //Serial.print(F("Logging to buffer: "));
      //updateSettings(a);
    }
    https.end();
  } else {
    Serial.println(F("HTTPS Unable to connect"));
    //Serial.print(F("Logging to buffer: "));
    //updateSettings(a);
  }
}

////// Miscellaneous functions
//// print variables for debug
void debug_vars() {
  Serial.println(F("- DEBUG -"));
  Serial.print(F("- hostname: "));
  Serial.println(hostname);
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
  Serial.print(F("- heater: "));
  Serial.println(heater);
  Serial.print(F("- manual: "));
  Serial.println(manual);
  if (emptyFile) {
    Serial.print(F("- emptyFile: "));
    Serial.println(emptyFile);
    return;
  }
  Serial.print(F("- SHA1: "));
  Serial.println(SHA1);
  Serial.print(F("- loghost: "));
  Serial.println(loghost);
  Serial.print(F("- httpsPort: "));
  Serial.println(httpsPort);
  Serial.print(F("- interval: "));
  Serial.println(interval);
  Serial.print(F("- temp_min: "));
  Serial.println(temp_min);
  Serial.print(F("- temp_max: "));
  Serial.println(temp_max);
  Serial.print(F("- MEM free heap: "));
  Serial.println(system_get_free_heap_size());
}

//// transform SHA1 to hex format needed for setFingerprint (from aa:ab:etc. to 0xaa, 0xab, etc.)
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

//// write to tft display
void updateDisplay() {
  tft.fillScreen(TFT_BLACK); // Black screen fill
  tft.setCursor(0, 0);
  tft.setTextFont(0);
  //tft.setFreeFont(0);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(1);
  tft.println("Connected to SSID:");
  tft.setTextSize(2);
  tft.println(WiFi.SSID());
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.println("LAN:" + String("") + lanIP);
  tft.println("Inet:" + inetIP);
  tft.println("Temperature:\n");
  tft.drawRoundRect(1, 52, 127, 22, 3, TFT_WHITE);
  if (temp_c < temp_min) {
    color = 0x001F;
  } else if (temp_c > temp_max) {
    color = 0xF800;
  } else {
    color = 0xFFE0;
  }
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.print(" ");
  tft.setTextSize(2);
  tft.print(String(temp_c, 2));
  tft.setTextSize(1);
  tft.print(" ");
  tft.setTextSize(2);
  int cursorX = tft.getCursorX() + 3;
  int cursorY = tft.getCursorY() + 4;
  tft.drawCircle(cursorX, cursorY, 3, color);
  //tft.print((char)247);
  tft.println(" C");
  tft.setTextSize(1);
  tft.println("");
  tft.setTextColor(TFT_WHITE);
  tft.println("MIN: " + String(temp_min, 1) + " " + (char)45 + " MAX: " + String(temp_max, 1) + "\n");
  tft.setTextSize(2);
  if (heater) {
    tft.setTextColor(TFT_RED);
    tft.print("Heater:");
  } else {
    tft.setTextColor(TFT_BLUE);
    tft.print("Cooler:");
  }
  if (relaisState == "ON") {
    tft.setTextColor(TFT_RED);
    tft.println(relaisState);
  } else {
    tft.setTextColor(TFT_GREEN);
    tft.println(relaisState);
  }
  if (manual) {
    mode = "Manual";
    tft.setTextColor(TFT_ORANGE);
  } else {
    mode = "Automatic";
    tft.setTextColor(TFT_GREEN);
  }
  tft.println(mode);
}

//// get internet IP (for tft.
void getInetIP() {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, "http://ipinfo.io/ip");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  if(httpCode > 0) {
    inetIP = String(http.getString());
    inetIP.trim();
  } else {
    Serial.print(F("HTTPS GET failed getting internet IP! Error: "));
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
  delay(50);
}

//// convert sizes in bytes to KB and MB
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

//// WiFi config mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Opening configuration portal");
  tft.fillScreen(TFT_BLACK); // Black screen fill
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(1);
  tft.println("WiFi Config\n");
  tft.setTextColor(TFT_RED);
  tft.print("IP:");
  tft.setTextSize(2);
  tft.println("10.0.1.1");
  tft.setTextSize(1);
  tft.print("ID:");
  tft.setTextSize(2);
  tft.println("Joey");
  tft.setTextSize(1);
  tft.print("Pwd: ");
  tft.setTextSize(2);
  tft.print("pass4esp");
}

//// setup / first run
void setup(void) {
  DS18B20.begin();
  Serial.begin(115200);
  tft.init();
  pinMode(RELAISPIN1, OUTPUT);
  pinMode(RELAISPIN2, OUTPUT);

  WiFiManager wifiManager;
  wifiManager.setTimeout(300);
  wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect("Joey","pass4esp")) {
    delay(3000);
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
    if (!MDNS.begin("esp8266"))
      Serial.println(F("Error setting up mDNS responder"));
  }
  //String WiFi_Name = WiFi.SSID();
  IPAddress ip = WiFi.localIP();
  sprintf(lanIP, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  getInetIP();

  SPIFFS.begin(); // initialize SPIFFS
  Serial.println("SPIFFS started. Contents:");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) { // List the file system contents
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
  }
  Serial.printf("\n");

  toggleRelais(0); // start with relais OFF
 
  readSettingsFile(); // read old settings from SPIFFS
  getTemperature();
  switchRelais();
  updateDisplay();
  if (debug)
    debug_vars();

  // web client handlers
  server.onNotFound(handleNotFound);
  server.on("/", handleRoot);
  server.on("/update", []() {
    updateSettings();
    getTemperature();
    switchRelais();
    updateDisplay();
    if (debug)
      debug_vars();
  });
  server.on("/clear", clearSettingsFile);

  updateDisplay();
  server.begin();
}
 
//// start main loop
void loop(void) {
  int hold = 1;
  while (digitalRead(TOUCHPIN1) == 1) { // hold here as long as sensor is touched to avoid switching on every loop pass
    if ( hold == 1) { // avoid switching more than once (hangs up the device)
      delay(200);
      if (digitalRead(TOUCHPIN2) == 1) { // if both touch sensors are triggered, switch back to auto mode
        manual = false;
        delay(200);
        hold = 0;
        Serial.println(F("\nSwitched to Automatic mode"));
        switchRelais();
        updateDisplay();
        writeSettingsFile();
        break;
      }
      toggleRelais(1);
      manual = true;
      updateDisplay();
      writeSettingsFile();
      Serial.println(F("\nSwitched to Manual mode, manually switched Relais ON"));
    }
    hold = 0;
  }

  // same comments as in previous 'while' apply here
  while (digitalRead(TOUCHPIN2) == 1) {
    if ( hold == 1) {
      delay(200);
      if (digitalRead(TOUCHPIN1) == 1) {
        manual = false;
        delay(200);
        hold = 0;
        Serial.println(F("\nSwitched to Automatic mode"));
        switchRelais();
        updateDisplay();
        writeSettingsFile();
        break;
      }
      toggleRelais(0);
      manual = true;
      updateDisplay();
      writeSettingsFile();
      Serial.println(F("\nSwitched to Manual mode, manually switched Relais OFF"));
    }
    hold = 0;
  }

  unsigned long currentMillis = millis();
  unsigned long past = currentMillis - previousMillis;
  if (past > interval) {
    Serial.println(F(" Interval passed"));
    previousMillis = currentMillis; // save the last time sensor was read

    if (emptyFile) {
      Serial.println(F("Switching relais off, as no temp_min/temp_max was set"));
      toggleRelais(0);
      Serial.println(F("Waiting for settings to be sent..."));
    } else {
      getTemperature();
      switchRelais();
      getInetIP();
      if (debug)
        debug_vars();
      updateDisplay();
      updateWebserver();
    }

    if (interval < 4999) { // set a failsafe interval
      interval = 10000;
    }
  } else {
    printProgress(past * 100 / interval);
  }
  server.handleClient();
}
