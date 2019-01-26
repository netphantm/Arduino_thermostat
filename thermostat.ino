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
#include "SSD1306Wire.h"
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

//// define variables / hardware
#define ONE_WIRE_BUS D4 // DS18B20 pin D4 = GPIO2
#define RELAISPIN D1 // or D0?
#define TOUCHPIN D0
#define PBSTR "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 79
//#define dsp_small

const size_t bufferSize = JSON_OBJECT_SIZE(6) + 160;
const static String sFile = "/settings.txt";
const static String configHost = "http://temperature.hugo.ro"; // chicken/egg situation, you have to get initial config somewhere
unsigned long uptime = (millis() / 1000 );
unsigned long prevTime = 0;
unsigned long prevTimeIP = 0;
bool emptyFile = false;
bool heater = true;
bool manual = false;
bool button;
bool debug = true;
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
uint16_t color;
int httpsPort;
int interval;
float temp_min;
float temp_max;
float temp_dev;

#if defined (dsp_small)
  String hostname = "Clamps";
  SSD1306Wire  tft(0x3c, D6, D5);
#else
  String hostname = "Joey";
  TFT_eSPI tft = TFT_eSPI();
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
ESP8266WebServer server(80);

//// read temperature from sensor / switch relay on or off
void getTemperature() {
  Serial.print("= getTemperature: ");
  String str_last = str_c;
  // read temperature from the sensor
  uptime = (millis() / 1000 ); // Refresh uptime
  delay(10);
  DS18B20.requestTemperatures();  // initialize temperature sensor
  temp_c = float(DS18B20.getTempCByIndex(0)); // read sensor
  yield();
  Serial.println(temp_c);
  temp_c = temp_c + temp_dev; // temperature deviation to calibrate your sensor
}

void toggleRelais(String sw = "TOGGLE") {
  Serial.print("= toggleRelais: ");
  if (sw == "TOGGLE") {
    if (digitalRead(RELAISPIN) == 1) {
      relaisState = "OFF";
      digitalWrite(RELAISPIN, 0);
    } else {
      relaisState = "ON";
      digitalWrite(RELAISPIN, 1);
    }
    Serial.println(relaisState);
    return;
  } else {
    if (sw == "ON") {
    relaisState = "ON";
    digitalWrite(RELAISPIN, 1);
    } else {
      relaisState = "OFF";
      digitalWrite(RELAISPIN, 0);
    }
    Serial.println(relaisState);
  }
}

void switchRelais() {
  if (manual) {
    return;
  } else {
    if (heater) {
      if (temp_c <= temp_min) {
        Serial.println("Auto turned relais ON");
        toggleRelais("ON");
      } else if (temp_c >= temp_max) {
        Serial.println("Auto turned relais OFF");
        toggleRelais("OFF");
      }
    } else {
      if (temp_c >= temp_max) {
        Serial.println("Auto turned relais ON");
        toggleRelais("ON");
      } else if (temp_c <= temp_min) {
        Serial.println("Auto turned relais OFF");
        toggleRelais("OFF");
      }
    }
  }
  delay(100);
}

//// settings read / write / clear SPIFFS
void clearSpiffs() {
  Serial.println("= clearSpiffs");

  Serial.println("Please wait for SPIFFS to be formatted");
  SPIFFS.format();
  Serial.println("SPIFFS formatted");
  yield();
  // mark file as empty
  emptyFile = true;
  server.send(200, "text/plain", "HTTP CODE 200: OK, SPIFFS formatted, settings cleared\n");
}

int readSettingsWeb() {
  Serial.println("= readSettingsWeb");
  String pathQuery = "/settings-";
  pathQuery += hostname;
  pathQuery += ".json";
  if (debug) {
    Serial.print(F("Getting settings from http://"));
    Serial.print(configHost);
    Serial.print(pathQuery);
  }
  WiFiClient client;
  HTTPClient http;
  http.begin(client, configHost + pathQuery);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  if(httpCode > 0) {
    String webJson = String(http.getString());
    //webJson.trim();
    if (debug) {
      Serial.print(F(": "));
      Serial.println(httpCode);
      Serial.print(F("Settings JSON from webserver: "));
      Serial.println(webJson);
    }
    deserializeJson(webJson);
  } else {
    Serial.print(F("HTTP GET failed getting settings from web! Error: "));
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
  return httpCode;
}

void readSettingsFile() {
  Serial.println("= readSettingsFile");

  File f = SPIFFS.open(sFile, "r"); // open file for reading
  // open file for reading
  if (!f) {
    Serial.println(F("Settings file read open failed"));
    emptyFile = true;
    return;
  }
  while (f.available()) {
    String fileJson = f.readStringUntil('\n');
    deserializeJson(fileJson);
  }
  f.close();
}

void deserializeJson(String json) {
  // deserialize JSON
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println(F("Error deserializing json!"));
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
    temp_dev = root["temp_dev"].as<float>(), sizeof(temp_dev);
    heater = root["heater"].as<bool>(), sizeof(heater);
    manual = root["manual"].as<bool>(), sizeof(manual);
    debug = root["debug"].as<bool>(), sizeof(debug);
    if (debug)
      Serial.println(F("Deserialized JSON"));
  }
}

void updateSettings() {
  Serial.println("\n= updateSettings");

  if (server.args() < 1 || server.args() > 10 || !server.arg("SHA1") || !server.arg("loghost")) {
    server.send(200, "text/html", "HTTP CODE 400: Invalid Request\n");
    return;
  }
  Serial.println("Got new settings");

  // get new settings from URL
  SHA1 = server.arg("SHA1");
  loghost = server.arg("loghost");
  httpsPort = server.arg("httpsPort").toInt();
  interval = server.arg("interval").toInt();
  temp_min = server.arg("temp_min").toFloat();
  temp_max = server.arg("temp_max").toFloat();
  temp_dev = server.arg("temp_dev").toFloat();
  heater = server.arg("heater").toInt() | 0;
  manual = server.arg("manual").toInt() | 0;
  debug = server.arg("debug").toInt() | 0;

  writeSettingsFile();
  server.send(200, "text/html", webString);
}

void writeSettingsFile() {
  Serial.print("= writeSettingsFile: ");

  File f = SPIFFS.open(sFile, "w"); // open file for writing
  if (!f) {
    Serial.println(F("Failed to create settings file"));
    server.send(200, "text/html", "HTTP CODE 200: OK, File write open failed! settings not saved\n");
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
  root["temp_dev"] = temp_dev;
  root["heater"] = heater;
  root["manual"] = manual;
  root["debug"] = debug;
  String outputJson;
  root.printTo(outputJson);
  if (debug) {
    Serial.print(F("- outputJson: "));
    Serial.println(outputJson);
  }

  // write JSON to file
  if (root.printTo(f) == 0) {
    Serial.println(F("Failed to write to file"));
    webString += "File write open failed\n";
  } else {
    Serial.println("OK");

    // prepare webpage for output
    webString = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n";
    webString += "<head>\n";
    webString += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n<style>\n";
    webString += "\tbody { \n\t\tpadding: 3rem; \n\t\tfont-size: 16px;\n\t}\n";
    webString += "\tform { \n\t\tdisplay: inline; \n\t}\n</style>\n";
    webString += "</head>\n<body>\n";
    webString += "HTTP CODE 200: OK, Got new settings<br>\n";
    webString += "Settings file updated.\n<br>\nBack to \n";
    webString += "<form method='POST' action='https://temperature.hugo.ro'>";
    webString += "\n<button name='device' value='";
    webString += hostname;
    webString += "'>Graph</button>\n";
    webString += "</form>\n<br>\n";
    webString += "JSON root: \n<br>\n";
    webString += "<div id='debug'></div>\n";
    webString += "<script src='https://temperature.hugo.ro/prettyprint.js'></script>\n";
    webString += "<script>\n\tvar root = ";
    webString += outputJson;
    webString += ";\n\tvar tbl = prettyPrint(root);\n";
    webString += "\tdocument.getElementById('debug').appendChild(tbl);\n</script>\n";
    webString += "</body>\n";
    emptyFile = false; // mark file as not empty
  }
  f.close();
  /*
  if (digitalRead(RELAISPIN) == 1) {
    relaisState = "ON";
  } else {
    relaisState = "OFF";
  }
  */
}

//// local webserver handlers / send data to logserver
void handleRoot() {
  server.send(200, "text/html", "<html><head></head><body><div align=\"center\"><h1>Nothing to see here! Move along...</h1></div></body></html>\n");
}

void handleNotFound(){
  server.send(404, "text/plain", "HTTP CODE 404: Not found\n");
}

void logToWebserver() {
  Serial.println("= logToWebserver");

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
  pathQuery += "&temp_dev=";
  pathQuery += temp_dev;
  pathQuery += "&interval=";
  pathQuery += interval;
  pathQuery += "&heater=";
  pathQuery += heater;
  pathQuery += "&manual=";
  pathQuery += manual;
  pathQuery += "&debug=";
  pathQuery += debug;

  if (debug) {
    Serial.print(F("Connecting to https://"));
    Serial.print(loghost);
    Serial.println(pathQuery);
  }

  BearSSL::WiFiClientSecure webClient;
  from_str();
  webClient.setFingerprint(sha1);
  HTTPClient https;
  if (https.begin(webClient, loghost, httpsPort, pathQuery)) {
    int httpCode = https.GET();
    if (httpCode > 0) {
      if (debug) {
        Serial.print(F("HTTPS GET OK, code: "));
        Serial.println(httpCode);
      }
      if (httpCode == HTTP_CODE_OK) {
          epochTime = https.getString();
          Serial.print("Timestamp on log update: ");
          Serial.println(epochTime);
      }
    } else {
      Serial.print(F("HTTPS GET failed! Error: "));
      Serial.println(https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.println(F("HTTPS Unable to connect"));
  }
}

////// Miscellaneous functions

//// print variables for debug
void debug_vars() {
  Serial.println(F("# DEBUG:"));
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
  Serial.print(F("- temp_dev: "));
  Serial.println(temp_dev);
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
#if defined (dsp_small)
    if (manual) {
      mode = "Manual";
    } else {
      mode = "Automatic";
    }
    String state = "Relais: " + relaisState + " / " + mode;
    tft.init();
    tft.setTextAlignment(TEXT_ALIGN_LEFT);
    tft.setFont(ArialMT_Plain_10);
    tft.drawString(0,  0, "Connected to WiFi");
    tft.drawString(0, 10, "SSID: " + WiFi.SSID());
    tft.drawString(0, 20, "LAN IP: " + String("") + lanIP);
    tft.drawString(0, 30, "Inet IP: " + inetIP);
    tft.drawString(0, 40, "Temperature: " + String(temp_c, 2) + "°C");
    tft.drawString(0, 50, state);
    tft.display();
#else
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextFont(0);
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
      color = 0xFFC8;
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
    int cursorX = tft.getCursorX() + 5;
    int cursorY = tft.getCursorY() + 2;
    tft.drawCircle(cursorX, cursorY, 2.5, color);
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
      tft.setTextColor(0xFFC8);
    } else {
      mode = "Automatic";
      tft.setTextColor(TFT_GREEN);
    }
    tft.println(mode);
#endif
}

//// get internet IP (for display)
void getInetIP() {
  Serial.print(F("= getInetIP"));
  unsigned long presTimeIP = millis();
  unsigned long pastIP = presTimeIP - prevTimeIP;
  if (presTimeIP < 60000 || pastIP > 3600000) { // update every hour, so we don't piss of the guys @ ipinfo.io
    WiFiClient client;
    HTTPClient http;
    http.begin(client, "http://ipinfo.io/ip");
    http.addHeader("Content-Type", "application/text");
    int httpCode = http.GET();
    if(httpCode > 0) {
      inetIP = String(http.getString());
      inetIP.trim();
      Serial.print(F(": "));
      Serial.print(inetIP);
      prevTimeIP = presTimeIP; // save the last time Ip was updated
    } else {
      Serial.print(F("HTTPS GET failed getting internet IP! Error: "));
      Serial.println(http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  Serial.println();
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
#if defined (dsp_small)
    tft.init();
    tft.setTextAlignment(TEXT_ALIGN_LEFT);
    tft.setFont(ArialMT_Plain_10);
    tft.drawString(0, 10, "# Entered config mode #");
    tft.drawString(0, 30, "ID/pass: NDND/pass4esp");
    tft.drawString(0, 40, "Config IP: 10.0.1.1");
    tft.display();
#else
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
#endif
}

//// setup / first run
void setup(void) {
  Serial.begin(115200);
  DS18B20.begin();
  delay(10);
  pinMode(RELAISPIN, OUTPUT);
  tft.init();

  WiFiManager wifiManager;
  wifiManager.setTimeout(300);
  wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect("Joey","pass4esp")) {
    delay(1000);
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

  SPIFFS.begin(); // initialize SPIFFS
  Serial.println("SPIFFS started. Contents:");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) { // List the file system contents
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
  }
  Serial.printf("\n");

  toggleRelais("OFF"); // start with relais OFF
 
  if (readSettingsWeb() != 200) // first, try reading settings from webserver
    readSettingsFile(); // if failed, read settings from SPIFFS
  getInetIP();
  getTemperature();
  switchRelais();
  updateDisplay();

  // local webserver client handlers
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
  server.on("/clear", clearSpiffs);
  server.begin();
}
 
//// start main loop
void loop(void) {
  bool hold = true;
  while (digitalRead(TOUCHPIN) == 1) { // hold here as long as sensor is touched to avoid switching on every loop pass
    button = true;
    if (hold) { // avoid switching more than once (hangs up the device)
      manual = true;
      delay(500);
      while (digitalRead(TOUCHPIN) == 1) { // if touch sensor is still triggered, switch back to auto mode
        manual = false;
        yield();
      }
    }
    yield();
    hold = false;
  }

  delay(100);
  if (button) {
    if (manual) {
      Serial.println(F("\nSwitched to Manual mode"));
      toggleRelais();
      updateDisplay();
    } else {
      Serial.println(F("\nSwitched to Automatic mode"));
      switchRelais();
      updateDisplay();
    }
    button = false;
  }

  unsigned long presTime = millis();
  unsigned long past = presTime - prevTime;
  if (past > interval) {
    Serial.println(F("\nInterval passed"));
    getInetIP();
    if (readSettingsWeb() != 200) // first, try reading settings from webserver
      readSettingsFile(); // if failed, read settings from SPIFFS
    prevTime = presTime; // save the last time sensor was read

    if (emptyFile) {
      Serial.println(F("Switching relais off, as no temp_min/temp_max was set"));
      toggleRelais("OFF");
      debug_vars();
      Serial.println(F("Waiting for settings to be sent..."));
    } else {
      getTemperature();
      switchRelais();
      if (debug)
        debug_vars();
      updateDisplay();
      logToWebserver();
    }

    if (interval < 4999) // set a failsafe interval
      interval = 10000;
  } else {
    printProgress(past * 100 / interval);
  }
  server.handleClient();
}
