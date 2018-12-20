<?php
print("<html><head>\n");
print("</head><body>\n");
print("<link rel=\"shortcut icon\" href=\"https://www.hugo.ro/favicon.ico\"/>\n");
print("<title>Grrrl Thermostat IoT Settings</title>\n");
print("<style>\n.content { background-color: lightcyan; width: 1000px; margin: auto; }</style>\n");
print("</head><body>\n<div class=\"content\">\n");
print("<form action=\"http://" + $IP + /update\" method=\"POST\">\n");
print("<div align=\"center\"><h1>ESP8266-12E DHT11 - Thermostat IoT - Settings</h1></div>\n");
print("<div align=\"center\"><table style=\"width:950px;\"><tr><td>");

print("IP = <input type='select' name='IP'>");
print(" <option value="">Select...</option>");
print(" <option value="192.168.178.104"></option>");
print("</select>");
print("Certificate Fingerprint SHA1 = <input type='text' name='SHA1' maxlength=59 size=55><br>");
print("Host = <input type='text' name='host' size=15>");
print("Port = <input type='text' name='httpsPort' size=5><br>");
print("Refresh interval = <input type='interval' name='SHA1' size=5><br>");
print("Temperature max = <input type='interval' name='temp_max' size=5><br>");
print("Temperature min = <input type='interval' name='temp_min' size=5><br>");
print("<input type='submit' value='Submit' >\n");

print("</body>");
