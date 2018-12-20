#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>
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
char buff_IP[16];
uint8_t sha1[20];
String webString = "<html><head></head><body><div align=\"center\"><h1>Nothing to see here! Move along...</h1></div></body></html>\n";
float temp_c;
String relaisState;
bool justFormatted = false;

//String json = "{\"SHA1\":\"7E:E0:9E:A8:24:61:96:DE:6C:20:3B:13:FC:17:57:AE:92:23:8D:48\",\"host\":\"temperature.hugo.ro\",\"httpsPort\":443,\"interval\":300000,\"temp_min\":6,\"temp_max\":12}";

String SHA1;
String host;
int httpsPort;
long interval;
float temp_min;
float temp_max;

ESP8266WebServer server(80);

void getTemperature() {
  uptime = (millis() / 1000 );
  DS18B20.requestTemperatures();  // initialize temperature sensor
  temp_c = float(DS18B20.getTempCByIndex(0));
  temp_c = temp_c - 2.4;
  delay(10);
}

void handle_root() {
  server.send(200, "text/html", webString);
}

void handleNotFound(){
  server.send(404, "text/plain", "HTTP CODE 404: Not found\n");
}

void clearPrefsFile() {
  File f = SPIFFS.open(pFile, "w");
  if (!f) {
    Serial.println("Preferences file clear OPEN FAILED!");
    server.send(200, "text/plain", "HTTP CODE 200: OK, File clear OPEN FAILED, NOT updated\n");
  } else {
    f.close();
    Serial.println("Preferences file cleared");
    server.send(200, "text/plain", "HTTP CODE 200: OK, Preferences file cleared\n");
  }
  delay(10);
}

void updatePrefsFile() {
  if (server.args() < 1 || server.args() > 6) {
    webString = "HTTP CODE 400: Invalid Request\n";
  } else {
    Serial.println("Got new preferences");
    webString = "HTTP CODE 200: OK, Got new Preferences\n";

    SHA1 = server.arg("SHA1");
    host = server.arg("host");
    httpsPort = server.arg("httpsPort").toInt();
    interval = server.arg("interval").toInt();
    temp_min = server.arg("temp_min").toInt();
    temp_max = server.arg("temp_max").toInt();

    File f = SPIFFS.open(pFile, "w");
    if (!f) {
      Serial.println(F("Failed to create file"));
      webString += "File write OPEN FAILED, NOT updated\n";
      server.send(200, "text/plain", webString);
      String webString = "<html><head></head><body><div align=\"center\"><h1>Nothing to see here! Move along...</h1></div></body></html>\n";
      return;
    }
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root["SHA1"] = SHA1;
    root["host"] = host;
    root["httpsPort"] = httpsPort;
    root["interval"] = interval;
    root["temp_min"] = temp_min;
    root["temp_max"] = temp_max;
    if (root.printTo(f) == 0) {
      Serial.println(F("Failed to write to file"));
      webString += "File write OPEN FAILED, NOT updated\n";
    } else {
      Serial.println("Preferences file updated");
      webString += "Preferences file updated\n";
      String output;
      root.printTo(output);
      webString += output;
    }
    f.close();
  }
  server.send(200, "text/plain", webString);
  String webString = "<html><head></head><body><div align=\"center\"><h1>Nothing to see here! Move along...</h1></div></body></html>\n";
  delay(10);
}

void readPrefsFile() {
  File f = SPIFFS.open(pFile, "r");
  if (!f) {
    Serial.println("Preferences file read OPEN FAILED!");
    return;
  }

  while (f.available()) {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(f);
    if (!root.success()) {
      Serial.println(F("Failed to deserialize json"));
      return;
    }

    SHA1 = root["SHA1"].as<String>();
    host = root["host"].as<String>(), sizeof(host);
    httpsPort = root["httpsPort"].as<int>(), sizeof(httpsPort);
    interval = root["interval"].as<long>(), sizeof(interval);
    temp_min = root["temp_min"].as<float>(), sizeof(temp_min);
    temp_max = root["temp_max"].as<float>(), sizeof(temp_max);
  }
  Serial.println("Got Preferences from file");

  f.close();
  delay(10);
}

void updateWebserver() {
  // configure url
  String url = "/logtemp.php?&status="; // <= mach OFF weg von hier!
  url += relaisState;
  url += "&uptime=";
  url += uptime;
  url += "&temperature=";
  url += temp_c;
  url += "&IP=";
  url += buff_IP;
  url += "&temp_min=";
  url += temp_min;
  url += "&temp_max=";
  url += temp_max;

  Serial.print("Connecting to https://");
  Serial.print(host);
  Serial.println(url);

  BearSSL::WiFiClientSecure webClient;
  from_str();
  webClient.setFingerprint(sha1);
  HTTPClient https;
  //Serial.println("[HTTPS] begin...");
  if (https.begin(webClient, host, httpsPort, url)) {  // HTTPS
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

void to_str() {
  SHA1 = "";
  for (int i = 0; i < 20; i++) {
    SHA1 = SHA1 + String(sha1[i], HEX);
    if (i < 19) {
      SHA1 = SHA1 + ":";
    }
  }
}

void from_str() {
  int j = 0;
  for (int i = 0; i < 60; i = i + 3) {
    String x = ("0x" + SHA1.substring(i, i+2));
    sha1[j] = strtoul(x.c_str(), NULL, 16);
    j++;
  }
}

void switchRelais() {
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
  delay(10);
}

void debug_vars() {
  Serial.print("");
  Serial.print("IP: ");
  Serial.println(buff_IP);
  Serial.print("uptime: ");
  Serial.println(uptime);
  Serial.print("Temperature: ");
  Serial.println(temp_c);
  Serial.print("RelaisState: ");
  Serial.println(relaisState);
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
  Serial.print("secs in void setup() connection: ");
  int connRes = WiFi.waitForConnectResult();
  Serial.println(connRes);
  if (WiFi.status()!=WL_CONNECTED) {
      Serial.println("Failed to connect, resetting in 5 seconds");
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
    Serial.println("Spiffs formatted");
  }
  f.close();

  pinMode(RELAISPIN1, OUTPUT);
  pinMode(RELAISPIN2, OUTPUT);
  digitalWrite(RELAISPIN1, LOW);
  digitalWrite(RELAISPIN2, LOW);
  relaisState = "OFF";
 
  getTemperature();
  switchRelais();

  // web client handlers
  server.on("/", handle_root);
  server.on("/update", updatePrefsFile);
  server.onNotFound(handleNotFound);
/*
  server.on("/clear", []() {
    clearPrefsFile();
  });
*/

  server.begin();
  Serial.println("HTTP Server started. Giving you 5 seconds to send data.");
  delay(5000);
} 
 
void loop(void) {

  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= interval )
  {
    // save the last time you read the sensor
    previousMillis = currentMillis;

    getTemperature();
    switchRelais();

    readPrefsFile();
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
