## DS18B20 thermostat
> WeMos D1 Mini Pro thermostat with DS18B20 oneWire sensor

![header](pics/header.png)

A thermostat using a DS18B20 oneWire sensor on a WeMos D1 Mini Pro. It uses two relays, for safety (to be sure that there is no current on either one of the power lines). I tried it first with an DHT11 shield, but that one is very unreliable, unstable and has a +/-2 &deg;C tolerance. The DS18B20 has a +/-0.5 &deg;C tolerance and is perfectly stable (and even cheaper). I couldn't find a shield with two relays, so I bought a kit and soldered it myself (circuit board, relays, terminal strip and SMD components).

The thermostat is made for a refrigerator, so it will fire up the relays if the temperature is greater than temp_max and power them off if temperature falls below temp_min (see TODO below).

All settings can be introduced in a form on the logging webserver. Some standard values are filled in, and the SHA1 fingerprint is calculated from the localhost certificate.

---

### Features:
- **Draws a google graph from the logged data (including a gauge with hysteresis and a toggle button for the date/time format of the graph)**
- **Uses a webserver to log the measurements (HTTPS)**
- **Compares the logservers certificate SHA1 fingerprint with the one provided in the settings**
- **Uses WiFiManager library, so you don't have to hardcode the WiFi credentials**
- **Shows time/date in the browser's timezone**
- **Provides a setup page for configuration variables that are sent to the WeMos, which then stores them as JSON to SPIFFS:**
    - WeMos D1 hostname (in case you have more than one device logging to the same webserver)
    - SHA1 fingerprint (calculated by the php script from localhosts certificate)
    - Logserver hostname
    - Logserver port (HTTPS)
    - Refresh/measuring interval
    - Minimum/maximum temperature values for the hysteresis

---

### Libraries used:
- <WiFiManager.h>
- <ESP8266HTTPClient.h>
- <ESP8266WebServer.h>
- <ESP8266mDNS.h>
- <FS.h>
- <OneWire.h>
- <DallasTemperature.h>
- <ArduinoJson.h>

---

### TODO (priority order may change):
- License, including links to the original inspiration sources (!ASAP!)
- Add a small TFT display for temperature, relay status and LAN IP
- Option to change between 'cooling' and 'heating' thermostats (fire up on temp_min or on temp_max)
- Set graphing time period (perhaps enable zooming)
- CSS styled mouseover tooltip, including relay status
- Perhaps: Change checking logserver validity from comparing SHA1 fingerprint, to check against uploaded CA certificate (longer validity then my letsencrypt signed certificate)
- Perhaps: switching to syslog on port 514 (UDP)
- Documentation (including hardware 'fritzing')
- Clean up code and add comments
- Add links to hardware (eBay)
- Change the horrible design
- Add some nice pictures √

---

### Links (hardware)

[16MB WeMos D1 Mini Pro # NodeMcu # ESP8266 ESP-8266EX CP2104 for Arduino NodeMCU](https://www.ebay.de/itm/16MB-WeMos-D1-Mini-Pro-NodeMcu-ESP8266-ESP-8266EX-CP2104-for-Arduino-NodeMCU/272405937539?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649)

[DS18b20 Wasserdichter Temperatursensor Thermometer Temperaturfühler LS4G](https://www.ebay.de/itm/DS18b20-Wasserdichter-Temperatursensor-Thermometer-Temperaturf%C3%BChler-LS4G/232860153205?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649)

[Dual Relais Shield für WEMOS D1 Mini (ESP 8266) WIFI Modul Kit/Bausatz -Neu!](https://www.ebay.de/itm/Dual-Relais-Shield-fur-WEMOS-D1-Mini-ESP-8266-WIFI-Modul-Kit-Bausatz-Neu/143054227109?_trkparms=aid%3D333200%26algo%3DCOMP.MBE%26ao%3D1%26asc%3D20140107084234%26meid%3Df7ec875706584a5497b0bdaa8d4e8cbc%26pid%3D100011%26rk%3D3%26rkt%3D12%26sd%3D143021212740%26itm%3D143054227109&_trksid=p2047675.c100011.m1850)

---

### Images

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-01.png" alt="pic-01" width="290px" height="210px"> <img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/header.png" alt="header" width="290px" height="185px"> <img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-02.png" alt="pic-02" width="290px" height="210px">

---

### Screenshot graphs

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-01.png" alt="screen-01">

### Screenshot setup
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-02.png" alt="screen-02">
