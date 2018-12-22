## DS18B20 thermostat
> WEMOS D1 Mini Pro thermostat with DS18B20 oneWire sensor

![header](header.png)

A thermostat using a DS18B20 oneWire sensor on a WEMOS D1 Mini Pro. It uses two relays, for safety (to be sure that there is no current on either one of the power lines). I tried it first with an DHT11 shield, but that one is very unreliable, unstable and has a +/-2 &deg;C tolerance. The DS18B20 has a +/-0.5 &deg;C tolerance and is perfectly stable (and even cheaper). I couldn't find a shield with two relays, so I bought a kit and soldered it myself (circuit board, relays, terminal strip and SMD components).

The thermostat is made for a refrigerator, so it will fire up the relays if the temperature is greater than temp_max and power them off if temperature falls below temp_min (see TODO below).

All settings can be introduced in a form on the logging webserver (with preloaded standard values).

---

### Features:
- **Draws a google graph from the logged data, including a gauge where you can see the hysteresis and a toggle for the date/time display**
- **Uses a webserver to log the measurements (HTTPS)**
- **Checks it's certificates SHA1 fingerprint against the one specified**
- **Uses WiFiManager lib, so you don't have to hardcode the WiFi credentials**
- **Shows time/date in the browser's timezone**
- **Setup page for configuration variables, which it then stores to SPIFFS in JSON format:**
    - WEMOS D1 hostname (in case you have more than one device logging to the same webserver)
    - SHA1 fingerprint (with php script running on the webserver, that reads it from localhost)
    - Log webserver hostname
    - Log webserver portname (HTTPS)
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

### TODO (order may change):
- Licensing, including links to the original inspiration sources (!ASAP!)
- Add a small TFT display for temperature, relay status and LAN IP
- Change checking logserver validity, from comparing SHA1 fingerprint to check uploaded and stored CA signing certificate (longer validity then my letsencrypt signed certificate)
- Option to change between 'cooling' and 'heating' thermostats (fire up on temp_min or on temp_max)
- Set graphing time period (perhaps enable zooming)
- CSS styled mouseover tooltip, including relay status
- Documentation (including hardware 'fritzing')
- Clean up code and add comments
- Add links to hardware (eBay)
- Change the horrible design
- Add some nice pictures

---

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pic-01.png" alt="pic-01" width="290px" height="210px"> <img src="https://github.com/netphantm/Arduino/raw/master/thermostat/header.png" alt="header" width="290px" height="185px"> <img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pic-02.png" alt="pic-02" width="290px" height="210px">
