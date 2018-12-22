## Relay socket with TFT display
> ESP8266 Socket relay switch

![](pics/pic-01.png)

An IoT socket with a WeMos D1 Mini Pro, an inductive touch sensor and a .96" OLED display. I made it, so I don't have to go in the evening all the way over to the nightstand to switch the light on, and go back to the desk and switch the light off (it's a 15m trip every evening :).

The first version is installed in a 3-way extension socket. It also inclded a touch sensor, so you can turn it on locally.

I used tzapu's WiFiManager, so you don't have to hardcode the WiFi credentials into the code (opening it up every time to flash the WeMos, if you decide to change your WiFi password or change location).

I've also used a simple relay for this one, since I didn't find a double relay shield at the time (for safety reasons, it's better to switch both power lines off). It should be simple to change though. Take a look at the other project called 'thermostat'.

To switch it on/off from my mobile, I use an app called 'HTTP Shortcuts'. You could also put a shortcut/link on your homescreen and fire up a browser every time, but that's not the simplest way to switch a light on. I suppose it could be integrated into [Home Assistant](https://hass.io/) or other (more fancy and expensive, the home spies you can talk to) home automation systems with some changes, but in my case that's overkill.

---

### Features
- **WiFiManager**
- **Touch sensor**
- **OLED Display
    - It Shows the socket status (On/Off), the IP of the own WiFi access point in config mode, and the LAN and internet IP after connecting to the local WiFi AP
    - The display dimms down, one minute after using the touch sensor

---

### Libraries needed

- ESP8266WiFi
- ESP8266WebServer
- WiFiManager
- ESP8266mDNS
- SSD1306Wire
- DNSServer
- ESP8266HTTPClient

---

### TODO
- [x] Not much, since it's already built into the box(es)

---

### Links

[16MB WeMos D1 Mini Pro # NodeMcu # ESP8266 ESP-8266EX CP2104 for Arduino NodeMCU](https://www.ebay.de/itm/16MB-WeMos-D1-Mini-Pro-NodeMcu-ESP8266-ESP-8266EX-CP2104-for-Arduino-NodeMCU/272405937539?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649)

[Relais Relay Shield für WeMos D1 mini Wifi ch340 IoT Lua Arduino kompatibel 397](https://www.ebay.de/itm/Relais-Relay-Shield-f%C3%BCr-WeMos-D1-mini-Wifi-ch340-IoT-Lua-Arduino-kompatibel-397/272273602704?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649)

[Dual Relais Shield für WEMOS D1 Mini (ESP 8266) WIFI Modul Kit/Bausatz -Neu!](https://www.ebay.de/itm/Dual-Relais-Shield-fur-WEMOS-D1-Mini-ESP-8266-WIFI-Modul-Kit-Bausatz-Neu/143054227109?_trkparms=aid%3D333200%26algo%3DCOMP.MBE%26ao%3D1%26asc%3D20140107084234%26meid%3Df7ec875706584a5497b0bdaa8d4e8cbc%26pid%3D100011%26rk%3D3%26rkt%3D12%26sd%3D143021212740%26itm%3D143054227109&_trksid=p2047675.c100011.m1850)

[ Details zu  0,96" OLED Display Weiß I2C SDD1306 128x64 Modul Arduino Raspberry Pi](https://www.ebay.de/itm/0-96-OLED-Display-Wei%C3%9F-I2C-SDD1306-128x64-Modul-Arduino-Raspberry-Pi/162465864403?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649)

[WiFiManager](https://github.com/tzapu/WiFiManager)

[Home Assistant](https://hass.io/)

---

### Images

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-01.png" alt="pic-01" width="440px"> <img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-02.png" alt="pic-02" width="440px">
<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-03.png" alt="pic-03" width="440px"> <img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/pic-04.png" alt="pic-04" width="440px">

### Screenshot

<img src="https://github.com/netphantm/Arduino/raw/master/thermostat/pics/screen-01.png" alt="screen-01">

---

### License

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](http://badges.mit-license.org)

- **[MIT license](http://opensource.org/licenses/mit-license.php)**
- Copyright 2018 © netphantm.

[↑ goto top](#DS18B20-thermostat)

