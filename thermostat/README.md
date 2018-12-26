## DS18B20 thermostat with logging
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/header.png" alt="header">

> ESP8266 thermostat with DS18B20 oneWire sensor

This is a simple thermostat, using a DS18B20 oneWire sensor on a WeMos D1 Mini Pro. It uses two relays, for safety reasons (to be sure that there is no current on either one of the power lines). I tried it first with an DHT11 shield, but that one is very unreliable, unstable and has a +/-2 &deg;C tolerance (and who needs a humidity sensor anyway, except if you use it in a sauna). The DS18B20 has a +/-0.5 &deg;C tolerance and is perfectly stable (and even cheaper). I couldn't find a shield with two relays, so I bought a kit and soldered it myself (circuit board, relays, terminal strip and the fun part: SMD components).

The thermostat is made for a refrigerator, so it will fire up the relays if the temperature is greater than temp_max and power them off if temperature falls below temp_min (see TODO below).

All variables can be initialized/changed in a settings form on the logging webserver. Some standard values are filled in and the SHA1 fingerprint is calculated from the localhost certificate, to make sure the WeMos sends the data to the right server.

TODO: Add two capacitive touch sensors to manually override relay state. One will turn them "ON" and the other one "OFF". If you press both, it switches back to auto mode. I don't have buttons, but it's way cooler with touch sensors. The other plus is that you don't have to bore holes in the inclosure for the buttons, just glue them on to the inner side and add stickers to the outside.

---

### Table of Contents
- [Features](#features)
- [Libraries](#Libraries-needed)
- [TODO](#TODO)
- [Links](#Links)
- [Images](#Images)
- [Screenshots](#Screenshot-graph-page)
- [License](#License)

[↑ goto top](#DS18B20-thermostat-with-logging)

---

### Features
- **Draws a google graph from the logged data (including a gauge with hysteresis and a toggle button for the date/time format of the graph)**
- **Uses a webserver to log the measurements (HTTPS)**
- **Compares the logservers certificate SHA1 fingerprint with the one provided in the settings**
- **Uses WiFiManager library, so you don't have to hardcode the WiFi credentials**
- **Shows date/time in the browser's timezone**
- **Provides a setup page for configuration variables that are sent to the WeMos, which then stores them as JSON to SPIFFS:**
    - WeMos D1 hostname (in case you have more than one device logging to the same webserver)
    - SHA1 fingerprint (calculated by the php script from localhosts certificate)
    - Logserver hostname
    - Logserver port (HTTPS)
    - Refresh/measuring interval
    - Minimum/maximum temperature values for the hysteresis

[↑ goto top](#DS18B20-thermostat-with-logging)

---

### Libraries needed
- WiFiManager
- ESP8266HTTPClient
- ESP8266WebServer
- ESP8266mDNS
- FS
- OneWire
- DallasTemperature
- ArduinoJson

[↑ goto top](#DS18B20-thermostat-with-logging)

---

### TODO
- [x] License, including links to the original inspiration sources (!ASAP!)
- [x] Checking that `temp_min <= temp_max` when new settings are pushed
- [x] Rechecking temperature when getting new preferences, instead of waiting for `interval` time to pass
- [x] Clean up code and add comments
- [x] Add a progress bar to the serial console, so you can predict how much until the next log update
- [ ] Change '192.168.178.104.php' to 'display.php' or something and select appliance from a drop-down menue if you have more than one
- [ ] Add a small TFT display for temperature, relay status and LAN IP
- [ ] Add manual override capacitive touch sensors
- [x] Option to change between _'cooler'_ and _'heater'_ thermostats (fire up on `temp >= temp_max` or on `temp <= temp_min`)
- [ ] CSS-styled mouseover tooltip for the graph, including relay status
- [x] Add links to hardware (eBay) and software (inspiration sources)
- [ ] Change the horrible design (now I've added even more of it)
- [x] Add some nice pictures
- [ ] \(Perhaps) Documentation, including 'fritzing'. In the mean time, look at my sources in the [Links (software)](#software) below
- [ ] \(Perhaps) Change checking logserver validity from comparing SHA1 fingerprint, to check against an uploaded CA certificate (longer validity then my letsencrypt signed certificate)
- [ ] \(Perhaps) Change the graph's time range (perhaps enable zooming)
- [ ] \(Perhaps) switching to syslog on port 514 (UDP?)

[↑ goto top](#DS18B20-thermostat-with-logging)

---

### Links

#### Hardware

- [16MB WeMos D1 Mini Pro # NodeMcu # ESP8266 ESP-8266EX CP2104 for Arduino NodeMCU](https://www.ebay.de/itm/16MB-WeMos-D1-Mini-Pro-NodeMcu-ESP8266-ESP-8266EX-CP2104-for-Arduino-NodeMCU/272405937539?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649)

- [DS18b20 Wasserdichter Temperatursensor Thermometer Temperaturfühler LS4G](https://www.ebay.de/itm/DS18b20-Wasserdichter-Temperatursensor-Thermometer-Temperaturf%C3%BChler-LS4G/232860153205?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649)

- [Dual Relais Shield für WEMOS D1 Mini (ESP 8266) WIFI Modul Kit/Bausatz -Neu!](https://www.ebay.de/itm/Dual-Relais-Shield-fur-WEMOS-D1-Mini-ESP-8266-WIFI-Modul-Kit-Bausatz-Neu/143054227109?_trkparms=aid%3D333200%26algo%3DCOMP.MBE%26ao%3D1%26asc%3D20140107084234%26meid%3Df7ec875706584a5497b0bdaa8d4e8cbc%26pid%3D100011%26rk%3D3%26rkt%3D12%26sd%3D143021212740%26itm%3D143054227109&_trksid=p2047675.c100011.m1850)

#### Software

- [WiFiManager](https://github.com/tzapu/WiFiManager)

- [Moment.js](https://momentjs.com/)

- [Adding WiFiManager](http://www.martyncurrey.com/esp8266-and-the-arduino-ide-part-5-adding-wifimanager/#more-7245)

- [ESP8266-12E-DHT-Thermostat](https://github.com/dmainmon/ESP8266-12E-DHT-Thermostat)

- [amake](https://github.com/pavelmc/amake)

[↑ goto top](#DS18B20-thermostat-with-logging)

---

### Images

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-01.png" alt="pic-01" width="290px" height="210px"> <img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-02.png" alt="pic-02" width="290px" height="210px"> <img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-03.png" alt="pic-03" width="290px" height="210px">

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-04.png" alt="pic-04" width="440px"><img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-05.png" alt="pic-05" width="440px">

[↑ goto top](#DS18B20-thermostat-with-logging)

### Screenshot graph page

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-01.png" alt="screen-01">

### Screenshot setup page
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-02.png" alt="screen-02">

### Screenshot console log
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-03.png" alt="screen-03">

### Screenshot device response
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-04.png" alt="screen-04">

[↑ goto top](#DS18B20-thermostat-with-logging)

---

### License

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](http://badges.mit-license.org)

- **[MIT license](http://opensource.org/licenses/mit-license.php)**
- Copyright 2018 © [Hugo](https://github.com/netphantm).

[↑ goto top](#DS18B20-thermostat-with-logging)

