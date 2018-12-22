String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String html_1 = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
    <meta charset='utf-8'>
    <style>
      body {font-size:140%; } 
      #main {display: table; margin: auto;  padding: 0 10px 0 10px; } 
      h2 {text-align:center; } 
      #RELAIS_button {
        background: #3D94F6;
        border-radius: 10px;
        color: #FFFFFF;
        font-family: Open Sans;
        width: 170px;
        font-size: 20px;
        padding: 15px;
        box-shadow: 2px 2px 5px black;
        text-shadow: 2px 2px 5px black;
        border: solid #337FED 1px;
      }
      .tooltip {
        position: fixed;
        font-family: Open Sans;
        font-size: 14px;
        bottom: 0;
        right: 0;
      }
      .tooltip .tooltiptext {
        opacity: 0;
        transition: opacity 1s;
        visibility: hidden;
        width: auto;
        background-color: black;
        color: #fff;
        text-align: center;
        border-radius: 3px;
        padding: 3px 0;
        position: absolute;
        z-index: 1;
        bottom: 100%;
        left: 50%;
        margin-left: -60px;
        opacity: 0;
        transition: opacity 1s;
      }
      .tooltip:hover .tooltiptext {
        visibility: visible;
        opacity: 1;
      }
      a:link {
        color: darkgray;
      }
    </style>
     
    <script>
      function switchRELAIS() {
         var button_text = document.getElementById("RELAIS_button").value;
         ajaxLoad('TOGGLE'); 
      }
    
      var ajaxRequest = null;
      if (window.XMLHttpRequest)  { ajaxRequest =new XMLHttpRequest(); }
      else                        { ajaxRequest =new ActiveXObject("Microsoft.XMLHTTP"); }
      
      function ajaxLoad(ajaxURL) {
        if(!ajaxRequest){ alert("AJAX is not supported."); return; }
        ajaxRequest.open("GET",ajaxURL,true);
        ajaxRequest.onreadystatechange = function() {
          if(ajaxRequest.readyState == 4 && ajaxRequest.status==200) {
            var ajaxResult = ajaxRequest.responseText;
            if      ( ajaxResult == "Relais is on" )   { document.getElementById("RELAIS_button").value = "Turn off"; }
            else if ( ajaxResult == "Relais is off" )  { document.getElementById("RELAIS_button").value = "Turn on"; }
            document.getElementById("reply").innerHTML = ajaxResult;
          }
        }
        ajaxRequest.send();
      }
    </script>
    <link rel="shortcut icon" href="https://www.hugo.ro/favicon.ico" />
    <title>Ndnd Power Control</title>
  </head>
  <body>
    <div id='main' align='center'>
      <h2>ESP8266 Power Control</h2>
      <div align="center">
        <p id = "reply">Reply appears here</p>
        <input type="button" id = "RELAIS_button" onclick="switchRELAIS()" value="Turn on" /> 
      </div>
    </div>
    <div class="tooltip"><a href="mailto:mail@hugo.ro?subject=Ndnd Power Control">©2018</a>
      <span class="tooltiptext">by Hugo and others</span>
    </div>
  </body>
</html>
)=====";

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <SSD1306Wire.h>
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>

#define RELAIS_PIN D1
#define TOUCH_PIN D7
SSD1306Wire  display(0x3c, D6, D5);

WiFiServer server(80);
String request = "";
int disp = 1;
long mins = (millis() / 60000);
long minsOld = mins;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
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

void setup() {
  pinMode(RELAIS_PIN, OUTPUT); 

  Serial.begin(115200);
  Serial.println("Serial started at 115200");
  Serial.println();

  WiFiManager wifiManager;
  wifiManager.setTimeout(300);
  wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect("NDND","pass4esp")) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }
  Serial.print(" secs in setup() connection result is ");
  int connRes = WiFi.waitForConnectResult();
  Serial.println(connRes);
  if (WiFi.status()!=WL_CONNECTED) {
      Serial.println("failed to connect, finishing setup anyway");
  } else {
    if (!MDNS.begin("esp8266"))   {  Serial.println("Error setting up MDNS responder!");  }
    else                          {  Serial.println("mDNS responder started");  }
    server.begin();
    Serial.println("Server started");
  }

  String WiFi_Name = WiFi.SSID();
  IPAddress ip = WiFi.localIP();
  char buf[16];
  sprintf(buf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  Serial.print("LAN IP: ");
  Serial.println(buf);

  String payload = "";
  HTTPClient http; // GET Inet IP
  http.begin("http://ipinfo.io/ip");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  if(httpCode == HTTP_CODE_OK) {
    Serial.print("HTTP response code ");
    Serial.println(httpCode);
    payload = http.getString();
    Serial.println("Inet IP: " + payload);
    display.init();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0,  0, "Connected to WiFi");
    display.drawString(0, 10, "WiFi AP SSID: " + WiFi_Name);
    display.drawString(0, 20, "LAN IP: " + String("") + buf);
    display.drawString(0, 30, "WAN IP: " + payload);
    display.display();
  }
  http.end();
  delay(50);
}

void loop() {
  mins = (millis() / 60000);
  int RELAIS_Pin = digitalRead(RELAIS_PIN);
  if (mins <= (minsOld + 1)) {
    display.normalDisplay();
    display.setBrightness(255);
    if (RELAIS_Pin == 0) {
      display.setColor(BLACK);
      display.drawString(0, 50, "Relais is ON");
      display.setColor(WHITE);
      display.drawString(0, 50, "Relais is OFF");
      display.display();
    } else if (RELAIS_Pin == 1){
      display.setColor(BLACK);
      display.drawString(0, 50, "Relais is OFF");
      display.setColor(WHITE);
      display.drawString(0, 50, "Relais is ON");
      display.display();
    }
  } else {
    display.setContrast(10,5,0);
  }

  int old = 0;
  while (digitalRead(TOUCH_PIN) == 1) {
    if ( old == 0) {
      if (RELAIS_Pin == 0) {
        digitalWrite(RELAIS_PIN, HIGH);
        Serial.println("Manually switched Relais ON");
      }
      else if (RELAIS_Pin == 1) {
        digitalWrite(RELAIS_PIN, LOW);;
        Serial.println("Manually switched Relais OFF");
      }
    }
    old = 1;
    minsOld = mins;
  }

  WiFiClient client = server.available(); // Check if a client has connected
  if (!client)  {  return;  }
  request = client.readStringUntil('\r'); // Read the first line of the request
  Serial.print("request: "); Serial.println(request); 
  if (request.indexOf("TOGGLE") > 0) {
    int RELAIS_Pin = digitalRead(RELAIS_PIN);
    if (RELAIS_Pin == 0) {
      digitalWrite(RELAIS_PIN, HIGH);
      client.print(header);
      client.print("Relais is on");
      Serial.println("Switched Relais ON");
    } else if (RELAIS_Pin == 1) {
      digitalWrite(RELAIS_PIN, LOW);
      client.print(header);
      client.print("Relais is off");
      Serial.println("Switched Relais OFF");
    }
  } else {
    client.flush();
    client.print(header);
    client.print(html_1);
    delay(50);
  }
}
