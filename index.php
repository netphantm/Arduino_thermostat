<?php

  // DEBUG
  //print("POST data: \n"); pr($_POST);

  $device = $_POST["device"];
  if (!$device) {
    $device = "Clamps";
  }

  if( isset($_POST['device']) && isset($_POST['uploadJson'])) {
    $settingsFileName = "settings-".$_POST['device'].".json";

    file_put_contents($settingsFileName, $_POST['uploadJson']);
    //error_log("written JSON to file ".$settingsFileName.": ".$_POST['uploadJson']."\n");
    print("written JSON to file ".$settingsFileName.": ".$_POST['uploadJson']."\n");
    exit(0);
  }

  function pr($var) {
    print '<pre>';
    print_r($var);
    print '</pre>';
  }

  function readDataFile() {
    if (isset($_POST['device'])) {
      $device = $_POST['device'];
    } else {
      $device = "Donbot";
    }
    if (empty($device)) {
      error_log("no \$device defined!");
      exit();
    }
    $searchString = ',';
    $numLines = 60;
    $maxLineLength = 100;
    $retStr = "";

    $logFileName = "temp-log-".$device.".csv";
    $logFile = fopen($logFileName, 'r');
    $data = fseek($logFile, -($numLines * $maxLineLength), SEEK_END);
    $lines = array();
    while (!feof($logFile)) {
      $lines[] = fgets($logFile);
    }
    //print pr($lines);

    $c = count($lines);
    $i = $c >= $numLines? $c-$numLines: 0;
    for (; $i<$c; ++$i) {
      if ($pos = strpos($lines[$i], $searchString)) {
        $line = str_getcsv($lines[$i],",","\"");
        //$uptime = $line[0];
        $state = $line[0];
        $temp = $line[1];
        $temp_min = $line[2];
        $temp_max = $line[3];
        $date = $line[4];
        $heater = ($line[5] == 1 ? true : false);
        $manual = ($line[6] == 1 ? true : false);
        $interval = ($line[7] / 60000);
        $temp_dev = isset($line[8]) ? floatval($line[8]) : 0;
        $retStr = $retStr."\n      [new Date(".$date."), '".$state."', ".$temp.", ".(($temp_max - $temp_min) / 2 + $temp_min)."], ";
      }
    }
    fclose($logFile);
    return array ($retStr, $date, $state, $temp_min, $temp_max, $temp, $heater, $manual, $interval, $temp_dev);
  }
?>

<html>
<head>
<style>
  * {
    margin: 0;
    padding: 0;
  }
  body {font-size:120%; }
  h2 {text-align:center; }
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
  .content {
    background-color: lightcyan;
    width: 1000px;
    margin: auto;
  }
  form {
    display: inline;
  }
</style>
<script src='moment.min.js' type='text/javascript'></script>
<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>
<script type='text/javascript'>
  google.charts.load('current', {'packages':['corechart','gauge']});
  google.charts.setOnLoadCallback(drawTempChart);
  google.charts.setOnLoadCallback(drawChart);

  function drawTempChart() {
    var data = google.visualization.arrayToDataTable([
      ['Label', 'Value'],
<?php
  print("      ['Temp °C', ".readDataFile()[5]."], ]);\n");
?>
    var options = {
      width: 250, height: 150,
      min: -10, max: 50,
<?php
  print("      greenFrom: -10, greenTo: ".readDataFile()[3].",\n");
  print("      yellowFrom: ".readDataFile()[3].", yellowTo: ".readDataFile()[4].",\n");
  print("      redFrom: ".readDataFile()[4].", redTo: 50,\n");
?>
      minorTicks: 5,
      focusTarget: 'category',
    };
    var chart = new google.visualization.Gauge(document.getElementById('chart_divTemp'));
    chart.draw(data, options);
  }

  function drawChart() {
    var data = google.visualization.arrayToDataTable([
      [{label: 'Date', type: 'datetime', role: 'domain'},
      {label: 'State', type: 'string', role: 'tooltip', 'p': {'html': true}},
      {label: 'Temperature', type: 'number', role: 'data'},
      {label: 'Temp_set', type: 'number', role: 'data'}],
<?php
  print(readDataFile()[0]);
?>
    ]);
    var options = {
      hAxis: {
        title: 'Date/Time',
        format: 'dd MMM, HH:mm',
      },
      vAxis: {
        title: 'Temperature (Celsius)',
      },
      series: {
        0: { color: 'blue', curveType: 'function' },
        1: { color: 'orange' },
      },
      animation: {
        startup: true,
        easing: 'out',
        duration: 1000,
      },
      tooltip: {
        isHtml: true,
        trigger: 'selection',
      },
      title: 'Temperature Activity',
      focustarget: 'category',
      backgroundColor: 'lightcyan',
      legend: { position: 'bottom' },
      chartArea: {width: '80%', height: '70%'},
      lineWidth: 2,
    };
    var date_formatter = new google.visualization.DateFormat({ pattern: 'dd MMM, HH:mm:ss' });
    date_formatter.format(data, 0);
    var chart = new google.visualization.LineChart(document.getElementById('curve_chart', 'tooltip_action'));
    chart.draw(data, options);
    var button = document.getElementById('change');
    button.onclick = function () {
      options.hAxis.format === 'dd MMM\nHH:mm' ?
      options.hAxis.format = 'dd/MM\nHH:mm' :
      options.hAxis.format = 'dd MMM\nHH:mm';
      chart.draw(data, options); 
    };
  }
</script>

<link rel='shortcut icon' href='https://www.hugo.ro/favicon.ico' />
<?php
  print("<title>".$device." - Thermostat IoT</title>");
?>
<!-- meta http-equiv='refresh' content='60' -->
</head>
<body>
<div class='content'>
<div align='center'><h2>ESP8266/WeMos D1 Mini Pro - DS18B20
<?php
  print("<br>".$device." - IoT Thermostat</h2></div>\n");
?>
<div align='center'><table style='width:950px;'><tr><td>
</td></tr></table>
<table style='width:950px;'><tr><td>
<div>Last Readings: <span id="displayMoment"></span></div>
<?php
  print("<div>Temperature: <b>".readDataFile()[5]." °C</b></div>\n");
  print("<div>Sensor deviation: <b>".readDataFile()[9]." °C</b></div>\n");
  print("<div>Hysteresis: <b>".readDataFile()[3]." °C > ".readDataFile()[4]." °C</b></div>\n");
  print("<div>Refresh interval: <b>".readDataFile()[8]."</b> minutes</div>\n");
  if (readDataFile()[6] == "1") {
    print("<div>Appliance is a <font style='color:red'><b>Heater</b></font></div>\n");
  } else {
    print("<div>Appliance is a <font style='color:blue'><b>Cooler</b></font></div>\n");
  }
  if (readDataFile()[2] == "OFF") {
    print("<div>Relais is: <font style='color:red'><b>OFF</b></font></div>\n");
  } else {
    print("<div>Relais is: <font style='color:green'><b>ON</b></font></div>\n");
  }
  if (readDataFile()[7] == "1") {
    print("<div>Mode: <font style='color:red'><b>Manual</b></font></div>\n");
  } else {
    print("<div>Mode: <font style='color:green'><b>Automatic</b></font></div>\n");
  }
?>
<form id='dev' method='POST'>
<?php
  print("Device hostname: ".$device."\n");
?>
  <select name='device' onchange='dev_change()'>
    <option>Select...</option>
    <option value='Clamps'>Clamps</option>
    <option value='Joey'>Joey</option>
    <option value='Donbot'>Donbot</option>
  </select>
</form>
<?php
  print("<form id='settings' method='POST' action='/settings.php'>");
  print("<input type='hidden' name='temp_min' value=".readDataFile()[3]." />\n");
  print("<input type='hidden' name='temp_max' value=".readDataFile()[4]." />\n");
  print("<input type='hidden' name='temp_dev' value=".readDataFile()[9]." />\n");
  print("<input type='hidden' name='heater' value=".(readDataFile()[6] ?: 0)." />\n");
  print("<input type='hidden' name='manual' value=".(readDataFile()[7] ?: 0)." />\n");
  print("<input type='hidden' name='interval' value=".readDataFile()[8]." />\n");
  print("<input type='hidden' name='device' value=".$device." />\n");
  print("<button name='device' value=".$device.">Settings</button>\n");
?>
</form></td><td>
<div id='chart_divTemp' style='width: 140px;'></div>
</td></tr></table>
<style>div.google-visualization-tooltip { ; }</style>
<div id='curve_chart' style='width: 1000px; height: 600px'></div></div>
<button id='change'>Change the date format</button>
<div class='tooltip'><a href='mailto:mail@hugo.ro?subject=Thermostat IoT'>&copy;2018</a>
<span class='tooltiptext'>by Hugo (and others)</span></div>
<script type='text/javascript'>
  function dev_change() {
    document.getElementById('dev').submit()
  }
  (function()
<?php
print("    { var Moment = moment.unix(".readDataFile()[1]."/1000).format('dddd, MMMM Do, YYYY HH:mm:ss');\n");
?>
      var eDisplayMoment = document.getElementById('displayMoment')
      eDisplayMoment.innerHTML = Moment;
    }
  )()
</script>
</body>
</html>
