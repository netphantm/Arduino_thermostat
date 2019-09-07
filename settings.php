<?php

//print "POST data: \n"; pr($_POST);

// needed for DEBUG
function pr($var) {
  print '<pre>';
  print_r($var);
  print '</pre>';
}

if( !empty($_POST["SHA1"]) && !empty($_POST["loghost"]) && !empty($_POST["httpsPort"]) && !empty($_POST["interval"]) && !empty($_POST["temp_min"]) && !empty($_POST["temp_max"]) ) {
  // gather data
  //$URL = $_POST["URL"];
  $SHA1 = $_POST["SHA1"];
  $loghost = $_POST["loghost"];
  $httpsPort = $_POST["httpsPort"];
  $interval = $_POST["interval"]*60000;
  $temp_min = $_POST["temp_min"];
  $temp_max = $_POST["temp_max"];
  $temp_dev = $_POST["temp_dev"];
  $heater = ($_POST["heater"] == "on" ? true : false);
  $manual = ($_POST["manual"] == "on" ? true : false);
  $debug = ($_POST["debug"] == "on" ? true : false);
  $hourP0 = isset($_POST["hourP0"]) ? $_POST["hourP0"] : 6;
  $minuteP0 = isset($_POST["minuteP0"]) ? $_POST["minuteP0"] : 30;
  $tempP0 = isset($_POST["tempP0"]) ? $_POST["tempP0"] : 23;
  $hourP1 = isset($_POST["hourP1"]) ? $_POST["hourP1"] : 9;
  $minuteP1 = isset($_POST["minuteP1"]) ? $_POST["minuteP1"] : 30;
  $tempP1 = isset($_POST["tempP1"]) ? $_POST["tempP1"] : 25;
  $hourP2 = isset($_POST["hourP2"]) ? $_POST["hourP2"] : 22;
  $minuteP2 = isset($_POST["minuteP2"]) ? $_POST["minuteP2"] : 30;
  $tempP2 = isset($_POST["tempP2"]) ? $_POST["tempP2"] : 21;

  if(($temp_min > ($temp_max - 1)) || ($interval < 30000) || ($interval > 1800000)) { // check if data is correct
    print("<html>\n");
    print("<body>\n");
    print("<link rel=\"shortcut icon\" href=\"https://www.hugo.ro/favicon.ico\"/>\n");
    print("<title>Thermostat IoT Settings</title>\n");
    print("<style>\n.content { background-color: red; width: 1000px; margin: auto; }</style>\n");
    print("</head><body>\n<div class=\"content\">\n");
    print("<div align=\"center\"><h2><p>Temperature MIN must be SMALLER then Temperature MAX and interval must be beween 2-30 minutes!</p></h2><h1><p>Try again!</p></h1></div>");
    print("</body>");
    exit();
  } else {
    // write JSON to file
    if ($_POST["device"] != "Donbot") {
      $array = Array (
        "SHA1" => $SHA1,
        "loghost" => $loghost,
        "httpsPort" => $httpsPort,
        "interval" => $interval,
        "temp_min" => $temp_min,
        "temp_max" => $temp_max,
        "temp_dev" => $temp_dev,
        "heater" => $heater,
        "manual" => $manual,
        "debug" => $debug,
      );
    } else {
      $arrayThermostat = Array (
        "SHA1" => $SHA1,
        "loghost" => $loghost,
        "httpsPort" => $httpsPort,
        "interval" => $interval,
        "temp_min" => $temp_min,
        "temp_max" => $temp_max,
        "temp_dev" => $temp_dev,
        "heater" => ($heater ? true : false),
        "manual" => ($manual ? true : false),
        "debug" => ($debug ? true : false),
      );
      $arrayPrograms = Array (
        array (
          "hour" => $hourP0,
          "minute" => $minuteP0,
          "temp" => $tempP0,
        ),
        array(
          "hour" => $hourP1,
          "minute" => $minuteP1,
          "temp" => $tempP1,
        ),
        array(
          "hour" => $hourP2,
          "minute" => $minuteP2,
          "temp" => $tempP2,
        ),
      );
      $array = Array (
        "thermostat" => $arrayThermostat,
        "programs" => $arrayPrograms,
      );
    }
    $json = json_encode($array);
    $fjson = "/var/www/temp/settings-".$_POST["device"].".json";
    if ($json) {
      file_put_contents($fjson, $json);
    } else {
      error_log("[/var/www/temp/settings.php] - Something went wrong, could not append to file /var/www/temp/settings-".$_GET["hostname"].".json");
    }

    // send data to device
    if ($_POST["device"] == "Donbot") {
      header('Location: '.$_POST["URL"]."update?SHA1=".$SHA1."&loghost=".$loghost."&httpsPort=".$httpsPort."&interval=".$interval."&temp_min=".$temp_min."&temp_max=".$temp_max."&temp_dev=".$temp_dev."&heater=".$heater."&manual=".$manual."&debug=".$debug."&hourP0=".$hourP0."&minuteP0=".$minuteP0."&tempP0=".$tempP0."&hourP1=".$hourP1."&minuteP1=".$minuteP1."&tempP1=".$tempP1."&hourP2=".$hourP2."&minuteP2=".$minuteP2."&tempP2=".$tempP2);
    } else {
      header('Location: '.$_POST["URL"]."update?SHA1=".$SHA1."&loghost=".$loghost."&httpsPort=".$httpsPort."&interval=".$interval."&temp_min=".$temp_min."&temp_max=".$temp_max."&temp_dev=".$temp_dev."&heater=".$heater."&manual=".$manual."&debug=".$debug);
    }
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
$loghost = "temperature.hugo.ro";
$httpsPort = "443";
$temp_min = isset($_POST["temp_min"]) ? $_POST["temp_min"] : 24;
$temp_max = isset($_POST["temp_max"]) ? $_POST["temp_max"] : 26;
$temp_dev = isset($_POST["temp_dev"]) ? $_POST["temp_dev"] : 0;
$interval = isset($_POST["interval"]) ? $_POST["interval"] : 300000;
$heater = ($_POST["heater"] == "on" ? true : false);
$heater = isset($_POST["heater"]) ? $_POST["heater"] : true;
$manual = isset($_POST["manual"]) ? $_POST["manual"] : false;
$debug = isset($_POST["debug"]) ? $_POST["debug"] : true;
if ($_POST["device"] == "Donbot") {
  $hourP0 = isset($_POST["hourP0"]) ? $_POST["hourP0"] : 6;
  $minuteP0 = isset($_POST["minuteP0"]) ? $_POST["minuteP0"] : 30;
  $tempP0 = isset($_POST["tempP0"]) ? $_POST["tempP0"] : 23;
  $hourP1 = isset($_POST["hourP1"]) ? $_POST["hourP1"] : 9;
  $minuteP1 = isset($_POST["minuteP1"]) ? $_POST["minuteP1"] : 30;
  $tempP1 = isset($_POST["tempP1"]) ? $_POST["tempP1"] : 25;
  $hourP2 = isset($_POST["hourP2"]) ? $_POST["hourP2"] : 21;
  $minuteP2 = isset($_POST["minuteP2"]) ? $_POST["minuteP2"] : 30;
  $tempP2 = isset($_POST["tempP2"]) ? $_POST["tempP2"] : 22;
}
print("<html>\n<head>\n");
?>
<?php
print("<link rel='shortcut icon' href='https://www.hugo.ro/favicon.ico'/>\n");
print("<title>IoT Thermostat - Settings</title>\n");
print("<style>\n.content { background-color: lightcyan; width: 1000px; margin: auto; }\n</style>\n");
print("</head>\n<body>\n<div class='content'>\n");
print("<div align=\"center\"><h2>ESP8266/WeMos D1 Mini Pro - DS18B20<br>");
print("IoT Thermostat - Settings</h2></div>\n");
print("<form method='POST'>\n");
print("Sensor hostname <select id='URL' name='URL' onchange='dev_change()'>\n");
print("<option value='http://192.168.178.103/'>Clamps</option>\n");
print("<option value='http://192.168.178.105/'>Joey</option>\n");
print("<option value='http://192.168.178.104/'>Donbot</option></select>\n");
print("<input type='hidden' name='device' value=".$_POST["device"]." />\n");
print("<br>Certificate SHA1 fingerprint <input type='text' name='SHA1' maxlength=60 size=42 value=$SHA1>\n");
print("<br>Loghost <input type='text' name='loghost' size=11 value=$loghost>\n");
print("Port <input type='text' name='httpsPort' size=2 value=$httpsPort></td></tr><tr><td>\n");
print("<br>Refresh interval <input type='text' name='interval' size=1 value=$interval> Minutes\n");
print("<br>Temperature MIN <input type='text' name='temp_min' size=1 value=$temp_min> &deg;C / \n");
print("Temperature MAX <input type='text' name='temp_max' size=1 value=$temp_max> &deg;C / \n");
print("Sensor deviation <input type='text' name='temp_dev' size=1 value=$temp_dev> &deg;C\n");
if ($_POST["device"] == "Donbot") {
  print("\t<input type='hidden' name='heater' value=on"); print(">\n");
  print("<br> <b>Program 1:</b> Starting hour <input type='text' name='hourP0' size=1 value=$hourP0> \n");
  print("Starting minute <input type='text' name='minuteP0' size=1 value=$minuteP0> \n");
  print("Temperature <input type='text' name='tempP0' size=1 value=$tempP0> &deg;C \n");
  print("<br><b> Program 2:</b> Starting hour <input type='text' name='hourP1' size=1 value=$hourP1> \n");
  print("Starting minute <input type='text' name='minuteP1' size=1 value=$minuteP1> \n");
  print("Temperature <input type='text' name='tempP1' size=1 value=$tempP1> &deg;C \n");
  print("<br><b> Program 3:</b> Starting hour <input type='text' name='hourP2' size=1 value=$hourP2> \n");
  print("Starting minute <input type='text' name='minuteP2' size=1 value=$minuteP2> \n");
  print("Temperature <input type='text' name='tempP2' size=1 value=$tempP2> &deg;C \n");
} else {
  print("<br>It's a Heater\t<input type='checkbox' name='heater'"); if ($heater) print(" checked=1"); print(">\n");
}
print("<br>Manual mode\t<input type='checkbox' name='manual'"); if ($manual) print(" checked=1"); print(">\n");
print("<br>[debug]\t<input type='checkbox' name='debug'"); if ($debug) print(" checked=1"); print(">\n");
print("<br><input type='submit' value='Submit' >\n");
print("</form></div></body>\n");
print("<script>\n");
print("function setSelectedIndex(s, v) {\n");
print("  for ( var i = 0; i < s.options.length; i++ ) {\n");
print("    if ( s.options[i].text == v ) {\n");
print("      s.options[i].selected = true;\n");
print("      return;\n");
print("    }\n");
print("  }\n");
print("}\n");
print("setSelectedIndex(document.getElementById('URL'),'".$_POST['device']."');\n");
print("</script>\n</html>\n");
?>
<script type='text/javascript'>
  function dev_change() {
    location.reload()
  }
</script>
