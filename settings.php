<?php

if( !empty($_GET["IP"]) && !empty($_GET["SHA1"]) && !empty($_GET["host"]) && !empty($_GET["httpsPort"]) && !empty($_GET["interval"]) && !empty($_GET["temp_min"]) && !empty($_GET["temp_max"]) ) {
  $IP = $_GET["IP"];
  $SHA1 = $_GET["SHA1"];
  $host = $_GET["host"];
  $httpsPort = $_GET["httpsPort"];
  $interval = $_GET["interval"];
  $temp_min = $_GET["temp_min"];
  $temp_max = $_GET["temp_max"];
  $heater = $_GET["heater"] == "true" ? true : false;
  print("HERE!");
  if($temp_min >= $temp_max) {
    print("<html>\n");
    print("<body>\n");
    print("<link rel=\"shortcut icon\" href=\"https://www.hugo.ro/favicon.ico\"/>\n");
    print("<title>Clamps Thermostat IoT Settings</title>\n");
    print("<style>\n.content { background-color: red; width: 1000px; margin: auto; }</style>\n");
    print("</head><body>\n<div class=\"content\">\n");
    print("<div align=\"center\"><h2><p>Temperature MIN must be SMALLER then Temperature MAX!</p></h2><h1><p>Try again!</p></h1></div>");
    print("</body>");
    exit();
  } else {
    header('Location: '.$_GET["IP"]."update?SHA1=".$SHA1."&host=".$host."&httpsPort=".$httpsPort."&interval=".$interval."&temp_min=".$temp_min."&temp_max=".$temp_max."&heater=".$heater);
    exit;
  }
}

function x509_fingerprint($pem,$hash='sha1')
{
  $pem = preg_replace('/\-+BEGIN CERTIFICATE\-+/','',$pem);
  $pem = preg_replace('/\-+END CERTIFICATE\-+/','',$pem);
  $pem = str_replace( array("\n","\r"), '', trim($pem));
  return strtoupper(hash($hash,base64_decode($pem)));
}

$contextOptions = array(
  'ssl' => array(
    'verify_peer' => false,
    'verify_peer_name' => false,
    #'allow_self_signed' => true,
    'disable_compression' => true,
    'capture_peer_cert' => true,
  )
);

$g = stream_context_create($contextOptions);
$r = stream_socket_client("ssl://localhost:443", $errno, $errstr, 20, STREAM_CLIENT_CONNECT, $g);
$cont = stream_context_get_params($r);
// Store remote certificate as a string
openssl_x509_export($cont['options']['ssl']['peer_certificate'], $x509_cert);

$pem = preg_replace( '/\-+(BEGIN|END) CERTIFICATE\-+/', '', $x509_cert);

$SHA1 = str_replace("SHA1 Fingerprint=", '', x509_fingerprint($pem,$hash='sha1'));
$SHA1 = wordwrap($SHA1 , 2 , ':' , true );
$host = "temperature.hugo.ro";
$httpsPort = "443";
$interval = 120000;
$temp_min = 6;
$temp_max = 12;
$heater = 0;

print("<html><head>\n");
print("</head><body>\n");
print("<link rel=\"shortcut icon\" href=\"https://www.hugo.ro/favicon.ico\"/>\n");
print("<title>Clamps Thermostat IoT Settings</title>\n");
print("<style>\n.content { background-color: lightcyan; width: 1000px; margin: auto; }</style>\n");
print("</head><body>\n<div class=\"content\">\n");
print("<form method=\"GET\">\n");
print("<div align=\"center\"><h1>ESP8266-12E DHT11 - Thermostat IoT - Settings</h1></div>\n");
print("<div align=\"center\"><table style=\"width:950px;\"><tr><td>\n");

print("Hostname = <select name=\"IP\">\n");#onchange=\"this.form.action=this.value
#print(" <option value=\"\">Select...</option>\n");
print(" <option value=\"http://192.168.178.104/\">Clamps</option>\n");
print("</select><br>\n");
print("Certificate Fingerprint SHA1 = <input type='text' name='SHA1' maxlength=59 size=40 value=$SHA1><br>\n");
print("Host = <input type='text' name='host' size=12 value=$host>\n");
print("Port = <input type='text' name='httpsPort' size=2 value=$httpsPort><br>\n");
print("Refresh interval = <input type='text' name='interval' size=2 value=$interval> Milliseconds<br>\n");
print("Temperature MIN = <input type='text' name='temp_min' size=1 value=$temp_min> &deg;C<br>\n");
print("Temperature MAX = <input type='text' name='temp_max' size=1 value=$temp_max> &deg;C<br>\n");
print("It's a Heater\t<input type='checkbox' name='heater' value='true'><br>\n");
print("<input type='submit' value='Submit' >\n");
print("</form></body>");
?>
