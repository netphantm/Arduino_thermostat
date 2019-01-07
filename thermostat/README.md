## DS18B20 thermostat with logging
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/header.png" alt="header">

> ESP8266 thermostat with DS18B20 oneWire sensor

This is a 'simple' thermostat controller, using a DS18B20 oneWire temperature sensor on a WeMos D1 Mini Pro. It uses two relays (for safety reasons, to be sure that there is no current on either one of the power lines). I tried it first with an DHT11 shield, but that one is very unreliable, unstable and has a +/-2 &deg;C tolerance (and who needs a humidity sensor anyway, except perhaps if you use it in a sauna). The DS18B20 has a +/-0.5 &deg;C tolerance and is perfectly stable (and even cheaper). I couldn't find a shield with two relays, so I bought a kit and soldered it myself (circuit board, relays, terminal strip and the fun part: SMD components).

The thermostat ~~is made for a refrigerator, so it~~ will fire up the relays if the temperature is greater than temp\_max and power them off if temperature falls below temp\_min if set as a _cooler_, and accordingly, fire up relays if temperature falls below temp\_min and power them off if the temperature is greater than temp\_max if set as a _heater_.

All necessary variables can be initialized/changed in a settings form on the logging webserver. Some standard values are filled in and the SHA1 fingerprint is calculated from the localhost certificate (to make sure the WeMos sends the data to the right logserver).

Two capacitive touch sensors can manually override relay state switching to manual mode. If you press both simultaneously, it switches back to auto mode and toggles relais status accordingly. I don't have buttons, but it's way cooler with touch sensors anyway. The other advantage is, you don't have to bore holes in the inclosure for the buttons. Just glue them to the inner side of the enclosure and add appropriate stickers to the outside.

---

### Table of Contents
- [Features](#features)
- [Libraries](#Libraries-needed)
- [TODO](#TODO)
- [Bugs](#Bugs)
- [Links](#Links)
- [Images](#Images)
- [Screenshots](#Screenshot-graph-page)
- [License](#License)
[↑ goto top](#DS18B20-thermostat-with-logging)

---

### Features
- **.96" TFT display for: SSID, LAN and internet IP, temperature, mode (man/auto) and relay status**
- **Uses WiFiManager library, so you don't have to hardcode the WiFi credentials**
- **Uses a webserver to log the measurements (HTTPS)**
- **Compares the logservers certificate SHA1 fingerprint with the one provided in the settings**
- **Progress bar on the serial console, showing time percentage until next log update**
- **Manual relay override capacitive touch sensors; press both to switch back to automatic mode**
- **Option to change between _'cooler'_ and _'heater'_ thermostats (fire up on `temp >= temp_max` or `temp <= temp_min`)**
- **Draws a google graph from the logged data and a gauge which shows hysteresis**
- **Shows last update date/time in the browser's timezone**
- **Provides a setup page for configuration variables that are sent to the WeMos, which then stores them as JSON to SPIFFS:**
    - WeMos D1 hostname (in case you have more than one device logging to the same webserver)
    - SHA1 fingerprint (calculated by the php script from localhosts certificate)
    - Logserver hostname
    - Logserver port (HTTPS)
    - Refresh/measuring interval
    - Minimum/maximum temperature values for the hysteresis
    - Thermostat type (cooler/heater)
    - Operation mode (manual/automatic)

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
- [x] Move some of these to the actual features section...
- [x] Add links to hardware (eBay) and software (inspiration sources)
- [x] Add some nice pictures
- [x] Checking that `temp_min <= temp_max` when new settings are pushed
- [x] Rechecking temperature when getting new preferences, instead of waiting for `interval` time to pass
- [x] Clean up code and add comments
- [x] Change '192.168.178.104.php' to 'index.php' and select appliance from a drop-down menue, if you have more than one
- [x] Add a settings button to the webpage, which opens the device's settings page
- [x] Change display to ST7735, this one is bigger and in color
- [ ] CSS-styled mouseover tooltip for the graph, including relay status
- [ ] Change the horrible design (now I've added even more of it)
- [ ] \(Perhaps)Write log to a local file if logserver is not reachable and resend it all when connectivity is restored (!)
- [ ] \(Perhaps) Documentation, including 'fritzing'. In the mean time, look at my sources in the [Links (software)](#software) below
- [ ] \(Perhaps) Change checking logserver validity from comparing SHA1 fingerprint, to check against an uploaded CA certificate (longer validity then a letsencrypt signed certificate)
- [ ] \(Perhaps) Change the graph's time range (perhaps enable zooming)
- [ ] \(Perhaps) switching to syslog on port 514 (UDP?)

[↑ goto top](#DS18B20-thermostat-with-logging)

---

### Bugs
- [ ] Display won't work if not initialized just before use (in the library examples, the init happens once in the setup() portion).

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

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-06.png" alt="pic-06" width="290px" height="210px"><img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-07.png" alt="pic-07" width="290px" height="210px">
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-08.png" alt="pic-08" width="290px" height="310px"><img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-09.png" alt="pic-09" width="290px" height="310px">

[↑ goto top](#DS18B20-thermostat-with-logging)

### Screenshot graph page

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-01.png" alt="screen-01">
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-02.png" alt="screen-02">

### Screenshot settings page
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-03.png" alt="screen-03">

### Screenshot console log
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-04.png" alt="screen-04">

### Screenshot device response
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-05.png" alt="screen-05">

### Screenshots device WiFi configuration
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-06.png" alt="screen-06" width="320px">
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-07.png" alt="screen-07" width="320px">

[↑ goto top](#DS18B20-thermostat-with-logging)

---

### License

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](http://badges.mit-license.org)

- **[MIT license](http://opensource.org/licenses/mit-license.php)**
- Copyright 2018 © [Hugo](https://github.com/netphantm).

[↑ goto top](#DS18B20-thermostat-with-logging)

