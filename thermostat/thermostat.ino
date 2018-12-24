#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

#define ONE_WIRE_BUS 2  // DS18B20 pin D4 = GPIO2
#define RELAISPIN1 D1
#define RELAISPIN2 D2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
const size_t bufferSize = JSON_OBJECT_SIZE(6) + 160;

const static String pFile = "/prefs.json";
unsigned long uptime = (millis() / 1000 );
unsigned long previousMillis = 0;
bool justFormatted = false;
bool emptyFile = false;
bool heater = false;
char buff_IP[16];
uint8_t sha1[20];
String webString = "<html><head></head><body><div align=\"center\"><h1>Nothing to see here! Move along...</h1></div></body></html>\n";
float temp_c;
String relaisState;
String SHA1;
String host;
int httpsPort;
long interval;
float temp_min;
float temp_max;

ESP8266WebServer server(80);

void getTemperature() {
  // read temperature from the sensor
  uptime = (millis() / 1000 );
  DS18B20.requestTemperatures();  // initialize temperature sensor
  temp_c = float(DS18B20.getTempCByIndex(0)); // read sensor
  temp_c = temp_c - 2.4; // calibrate sensor
  delay(10);
  switchRelais();
}

void handle_root() {
  server.send(200, "text/html", webString);
}

void handleNotFound(){
  server.send(404, "text/plain", "<h2><font style='color:red'>HTTP CODE 404: Not found</font></h2>\n");
}

void clearPrefsFile() {
  Serial.println("Please wait for SPIFFS to be formatted");
  SPIFFS.format();
  Serial.println("SPIFFS formatted");
  delay(10);
  // mark file as empty
  emptyFile = true;
  server.send(200, "text/plain", "<h2><font style='color:green'>[HTTPS] OK, SPIFFS formatted, preferences cleared</font></h2>");
}

void updatePrefsFile() {
  if (server.args() < 1 || server.args() > 7) {
    webString = "<h2><font style='color:red'>HTTP CODE 400: Invalid Request</font></h2>\n";
  } else {
    Serial.println("Got new preferences");
    webString = "<h2><font style='color:green'>HTTP CODE 200: OK, Got new Preferences</font></h2>\n";

    // get new settings from URL
    SHA1 = server.arg("SHA1");
    host = server.arg("host");
    httpsPort = server.arg("httpsPort").toInt();
    interval = server.arg("interval").toInt();
    temp_min = server.arg("temp_min").toInt();
    temp_max = server.arg("temp_max").toInt();
    heater = server.arg("heater").toInt();

    // open file for writing
    File f = SPIFFS.open(pFile, "w");
    if (!f) {
      Serial.println(F("Failed to create file"));
      server.send(200, "text/plain", "<h2><font style='color:red'>File write OPEN FAILED, NOT updated</font></h2>\n");
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

    // write JSON to file
    if (root.printTo(f) == 0) {
      Serial.println(F("Failed to write to file"));
      webString = "<h2><font style='color:red'>File write OPEN FAILED, NOT updated</font></h2>\n";
    } else {
      Serial.println("Preferences file updated");
      webString = "<h2><font style='color:green'>Preferences file updated\n";
      String output;
      root.printTo(output);
      webString += output;
      webString += "</font></h2>";
      // mark file as not empty
      emptyFile = false;
    }
    f.close();
  }
  delay(10);
  server.send(200, "text/plain", webString);
  // reset webString for normal / output
  String webString = "<html><head></head><body><div align=\"center\"><h1>Nothing to see here! Move along...</h1></div></body></html>\n";
}

void readPrefsFile() {
  File f = SPIFFS.open(pFile, "r");
  // open file for reading
  if (!f) {
    Serial.println("Preferences file read OPEN FAILED!");
    emptyFile = true;
    return;
  }
  while (f.available()) {
    // deserialize JSON
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(f);
    if (!root.success()) {
      Serial.println(F("ERROR deserializing json, maybe you cleared the preferences file? Not updating logserver!"));
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
      temp_max = root["heater"].as<bool>(), sizeof(heater);
      Serial.println("Got Preferences from file");
    }
  }
  f.close();
  delay(10);
}

void updateWebserver() {
  // configure path + query for sending to logserver
  if (emptyFile) {
    Serial.println(F("ERROR deserializing json, maybe you cleared the preferences file? Not updating logserver!"));
    return;
  }
  String pathQuery = "/logtemp.php?&status=";
  pathQuery += relaisState;
  pathQuery += "&uptime=";
  pathQuery += uptime;
  pathQuery += "&temperature=";
  pathQuery += temp_c;
  pathQuery += "&IP=";
  pathQuery += buff_IP;
  pathQuery += "&temp_min=";
  pathQuery += temp_min;
  pathQuery += "&temp_max=";
  pathQuery += temp_max;
  pathQuery += "&heater=";
  pathQuery += heater;

  Serial.print("Connecting to https://");
  Serial.print(host);
  Serial.println(pathQuery);

  BearSSL::WiFiClientSecure webClient;
  webClient.setFingerprint(sha1);
  HTTPClient https;
  //Serial.println("[HTTPS] begin...");
  if (https.begin(webClient, host, httpsPort, pathQuery)) {  // HTTPS
    //Serial.println("[HTTPS] GET...");
    int httpCode = https.GET();
    if (httpCode > 0) {
      Serial.print("[HTTPS] GET OK, code: ");
      Serial.println(httpCode);
      if (httpCode == HTTP_CODE_OK) {
          String payload = https.getString();
          Serial.println(payload);
      }
    } else {
      Serial.print("[HTTPS] GET FAILED! Error: ");
      Serial.println(https.errorToString(httpCode).c_str());
    }
    //Serial.println("closing connection");
    https.end();
  } else {
    Serial.println("[HTTPS] Unable to connect");
  }
}

void switchRelais() {
  if (heater) {
    if (temp_c <= temp_min) {
      digitalWrite(RELAISPIN1, HIGH);
      digitalWrite(RELAISPIN2, HIGH);
      Serial.println("Turn both Relais ON");
      relaisState = "ON";
    } else if (temp_c >= temp_max) {
      digitalWrite(RELAISPIN1, LOW);
      digitalWrite(RELAISPIN2, LOW);
      Serial.println("Turn both Relais OFF");
      relaisState = "OFF";
    }
  } else {
    if (temp_c >= temp_max) {
      digitalWrite(RELAISPIN1, HIGH);
      digitalWrite(RELAISPIN2, HIGH);
      Serial.println("Turn both Relais ON");
      relaisState = "ON";
    } else if (temp_c <= temp_min) {
      digitalWrite(RELAISPIN1, LOW);
      digitalWrite(RELAISPIN2, LOW);
      Serial.println("Turn both Relais OFF");
      relaisState = "OFF";
    }
  }
  delay(10);
}

void debug_vars() {
  // write debug to serial port
  Serial.print("");
  Serial.print("IP: ");
  Serial.println(buff_IP);
  Serial.print("uptime: ");
  Serial.println(uptime);
  Serial.print("Temperature: ");
  Serial.println(temp_c);
  Serial.print("RelaisState: ");
  Serial.println(relaisState);
  if (emptyFile) {
    Serial.print("emptyFile: ");
    Serial.println(emptyFile);
    return;
  }
  Serial.print("SHA1: ");
  Serial.println(SHA1);
  Serial.print("host: ");
  Serial.println(host);
  Serial.print("httpsPort: ");
  Serial.println(httpsPort);
  Serial.print("interval: ");
  Serial.println(interval);
  Serial.print("temp_min: ");
  Serial.println(temp_min);
  Serial.print("temp_max: ");
  Serial.println(temp_max);
  Serial.print("heater: ");
  Serial.println(heater);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println("Opening configuration portal");
}

void setup(void) {
  Serial.begin(115200); // Start Serial 
 
  WiFiManager wifiManager;
  wifiManager.setTimeout(300);
  wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect("Clamps","pass4esp")) {
    delay(5000);
    Serial.println("Failed to connect and hit timeout, restarting...");
    ESP.reset();
  }
  Serial.print("Seconds in void setup() WiFi connection: ");
  int connRes = WiFi.waitForConnectResult();
  Serial.println(connRes);
  if (WiFi.status()!=WL_CONNECTED) {
      Serial.println("Failed to connect to WiFi, resetting in 5 seconds");
      delay(5000);
      ESP.reset();
  } else {
    if (!MDNS.begin("esp8266"))   {  Serial.println("Error setting up MDNS responder!");  }
    else                          {  Serial.println("mDNS responder started");  }
  }
  String WiFi_Name = WiFi.SSID();
  IPAddress ip = WiFi.localIP();
  sprintf(buff_IP, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  Serial.print("LAN IP: ");
  Serial.println(buff_IP);

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
      Serial.println("Please wait for SPIFFS to be formatted");
      SPIFFS.format();
    }
    Serial.println("SPIFFS formatted");
  }
  f.close();

  pinMode(RELAISPIN1, OUTPUT);
  pinMode(RELAISPIN2, OUTPUT);
  // start with relais OFF
  digitalWrite(RELAISPIN1, LOW);
  digitalWrite(RELAISPIN2, LOW);
  relaisState = "OFF";
 
  Serial.println("- getTemperature");
  getTemperature();

  // web client handlers
  server.onNotFound(handleNotFound);
  server.on("/", handle_root);
  server.on("/update", []() {
    updatePrefsFile();
    getTemperature();
    debug_vars();
  });
  server.on("/clear", []() {
    clearPrefsFile();
  });

  server.begin();
  Serial.println("HTTP Server started. Giving you 5 seconds to send data.");
  delay(5000);
} 
 
void loop(void) {
  // start main loop
  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= interval )
  {
    // save the last time sensor was read
    previousMillis = currentMillis;

    Serial.println("- getTemperature");
    getTemperature();
    Serial.println("- readPrefsFile");
    readPrefsFile();
    Serial.println("- updateWebserver");
    updateWebserver();

    // Failsafe, if json was malformed:
    if (interval < 5000) {
      interval = 120000;
    }
    debug_vars();
  }
  delay(10);
  server.handleClient();
}
