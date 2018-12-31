<?php

// renders the temp from the temperature log from 'Clamps' or other device
// using a nice google gauge and an even nicer google chart

if (session_status() == PHP_SESSION_NONE  || session_id() == '') {
  session_start();
}

if (empty($_POST['device'])) {
  $_SESSION['device'] = 'Clamps';
} else {
  $_SESSION['device'] = $_POST['device'];
}

header("refresh:300; url=index.php", FALSE, 307);

//pr($_SESSION);

// needed for DEBUG
function pr($var) {
  print '<pre>';
  print_r($var);
  print '</pre>';
}

function readDataFile() {
  switch ($_SESSION['device']) {
    case "Clamps":
      $filename = 'temp-log-192.168.178.104.csv';
      break;
    case "Joey":
      $filename = 'temp-log-192.168.178.105.csv';
      break;
    case "Donbot":
      $filename = 'temp-log-192.168.178.106.csv';
      break;
  }

  $searchString = ',';
  $numLines = 60;
  $maxLineLength = 100;
  $retStr = "";

  $fp = fopen($filename, 'r');
  $data = fseek($fp, -($numLines * $maxLineLength), SEEK_END);
  $lines = array();
  while (!feof($fp)) {
    $lines[] = fgets($fp);
  }
  #print pr($lines);

  $c = count($lines);
  $i = $c >= $numLines? $c-$numLines: 0;
  for (; $i<$c; ++$i) {
    if ($pos = strpos($lines[$i], $searchString)) {
      #echo $lines[$i];
      $line = str_getcsv($lines[$i],",","\"");
      $uptime = $line[0];
      $state = $line[1];
      $temp = $line[2];
      $temp_min = $line[3];
      $temp_max = $line[4];
      $date = $line[5];
      $heater = $line[6];
      $manual = $line[7];

      //$retStr = $retStr."\n      [new Date(".$date."), ".$temp.", '<table><tr><td>".options.hAxis."</td></tr><tr><td>Temp: ".$temp."</td></tr><tr><td>State: ".$state."</td></tr></table>'], ";
      $retStr = $retStr."\n      [new Date(".$date."), '".$state."', ".$temp."], ";
    }
  }
  fclose($fp);
  return array ($retStr, $date, $state, $temp_min, $temp_max, $temp, $heater, $manual);
}

////////////////////////////////////////////////////////////////
//      creates the HTML string to be sent to the client      //
////////////////////////////////////////////////////////////////

  print("<html><head>\n");
  #print(pr(readDataFile()));
  print("<style>\n");
  print("  * {\n");
  print("      margin: 0;\n");
  print("      padding: 0;\n");
  print("  }\n");
  print("  body {font-size:120%; } \n");
  print("  h2 {text-align:center; } \n");
  print("  .tooltip {\n");
  print("    position: fixed;\n");
  print("    font-family: Open Sans;\n");
  print("    font-size: 14px;\n");
  print("    bottom: 0;\n");
  print("    right: 0;\n");
  print("  }\n");
  print("  .tooltip .tooltiptext {\n");
  print("    opacity: 0;\n");
  print("    transition: opacity 1s;\n");
  print("    visibility: hidden;\n");
  print("    width: auto;\n");
  print("    background-color: black;\n");
  print("    color: #fff;\n");
  print("    text-align: center;\n");
  print("    border-radius: 3px;\n");
  print("    padding: 3px 0;\n");
  print("    position: absolute;\n");
  print("    z-index: 1;\n");
  print("    bottom: 100%;\n");
  print("    left: 50%;\n");
  print("    margin-left: -60px;\n");
  print("    opacity: 0;\n");
  print("    transition: opacity 1s;\n");
  print("  }\n");
  print("  .tooltip:hover .tooltiptext {\n");
  print("    visibility: visible;\n");
  print("    opacity: 1;\n");
  print("  }\n");
  print("  a:link {\n");
  print("    color: darkgray;\n");
  print("  }\n");
  print("</style>\n");
  //print("<meta http-equiv='refresh' content='60'> \n");
  print("<script src='moment.min.js' type='text/javascript'></script>\n");

  ///////////// google charts script
  // setup the google gauge options here
  print("<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script> \n");
  print("<script type='text/javascript'> \n");
  print("  google.charts.load('current', {'packages':['corechart','gauge']}); \n");
  print("  google.charts.setOnLoadCallback(drawTempChart); \n");
  print("  google.charts.setOnLoadCallback(drawChart); \n ");

  // draw temp gaug)e
  print("  function drawTempChart() { \n");
  print("    var data = google.visualization.arrayToDataTable([ \n");
  print("      ['Label', 'Value'], \n");
  print("      ['Temp °C', ");
  print(readDataFile()[5]);
  print("], ]); \n");

  // setup the google gauge options here
  print("    var options = { \n");
  print("      width: 250, height: 150, \n");
  print("      min: 0, max: 40, \n");
  print("      greenFrom: 0, greenTo: ".readDataFile()[3].", \n");
  print("      yellowFrom: ".readDataFile()[3].", yellowTo: ".readDataFile()[4].", \n");
  print("      redFrom: ".readDataFile()[4].", redTo: 40, \n");
  print("      minorTicks: 5, \n");
  print("      focusTarget: 'category', \n");
  print("    }; \n");
  print("    var chart = new google.visualization.Gauge(document.getElementById('chart_divTemp')); \n");
  print("    chart.draw(data, options); \n");
  print("  } \n");

  // draw main graph
  print("  function drawChart() { \n");
  print("    var data = google.visualization.arrayToDataTable([ \n");
  print("      [{label: 'Date', type: 'datetime', role: 'domain'}, \n");
  print("      {label: 'State', type: 'string', role: 'tooltip', 'p': {'html': true}}, \n");
  print("      {label: 'Temperature', type: 'number', role: 'data'}], \n");

  // open file for reading
  print(readDataFile()[0]);

  print("\n    ]); \n");
  print("    var options = { \n");
  print("      hAxis: { \n");
  print("        title: 'Date/Time', \n"); // horizontal axis text
  print("        format: 'dd MMM, HH:mm', \n");
  print("      }, \n");
  print("      vAxis: { \n");
  print("        title: 'Temperature (Celsius)', \n"); // vertical axis text
  print("      }, \n");
  print("      animation: { \n");
  print("        startup: true, \n");
  print("        easing: 'out', \n");
  print("        duration: 1000, \n");
  print("      }, \n");
  print("      tooltip: { \n");
  print("        isHtml: true, \n");
  //print("        trigger: 'selection', \n");
  print("      }, \n");
  print("      title: 'Temperature Activity', \n");
  print("      focustarget: 'category', \n");
  print("      curveType: 'function', \n");
  print("      backgroundColor: 'lightcyan', \n");
  print("      legend: { position: 'bottom' }, \n");
  print("      chartArea: {width: '80%', height: '70%'}, \n");
  print("      lineWidth: 2, \n");
  //print("      crosshair: { trigger: 'focus' }, \n");
  print("    }; \n");
  print("    var date_formatter = new google.visualization.DateFormat({ pattern: 'dd MMM, HH:mm:ss' }); \n");
  print("    date_formatter.format(data, 0); \n");
  print("    var chart = new google.visualization.LineChart(document.getElementById('curve_chart', 'tooltip_action')); \n");
  print("    chart.draw(data, options); \n");
  print("    var button = document.getElementById('change'); \n");
  print("    button.onclick = function () { \n");
  print("      options.hAxis.format === 'dd MMM, HH:mm' ? \n");
  print("      options.hAxis.format = 'd/M-HH:mm' : \n");
  print("      options.hAxis.format = 'dd MMM, HH:mm'; \n");
  print("      chart.draw(data, options); \n");
  print("    }; \n");
  print("  } \n");
  print("</script> \n");
  //////////// HTML
  print("<link rel='shortcut icon' href='https://www.hugo.ro/favicon.ico'/>\n");
  print("<title>".$_SESSION['device']." - Thermostat IoT</title>\n");
  print("<style>\n.content { background-color: lightcyan; width: 1000px; margin: auto; }</style>\n");
  print("</head><body>\n");

  print("\n<div class='content'>\n");
  print("<div align='center'><h2>ESP8266/WeMos D1 Mini Pro - DS18B20");
  print("<br>".$_SESSION['device']." - IoT Thermostat</h2></div>\n");
  print("<div align='center'><table style='width:950px;'><tr><td>");
  print("</td></tr></table>\n");
  print("<table style='width:950px;'><tr><td>\n");
  print("<div>Last Readings: ");
  print("<span id=\"displayMoment\"></span>");
  print("</div>\n");
  print("<div>Temperature: <b>".readDataFile()[5]." °C</b></div>\n");
  print("<div>Hysteresis: <b>".readDataFile()[3]." °C - ".readDataFile()[4]." °C</b></div>\n");
  if (readDataFile()[7] == "1") {
    print("<div>Mode: <font style='color:red'><b>Manual</b></font></div>\n");
  } else {
    print("<div>Mode: <font style='color:green'><b>Automatic</b></font></div>\n");
  }
  if (readDataFile()[2] == "OFF") {
    print("<div>Relais is: <font style='color:red'><b>OFF</b></font></div>\n");
  } else {
    print("<div>Relais is: <font style='color:green'><b>ON</b></font></div>\n");
  }
  if (readDataFile()[6] == "1") {
    print("<div>Appliance is a <font style='color:red'><b>Heater</b></font></div>\n");
  } else {
    print("<div>Appliance is a <font style='color:blue'><b>Cooler</b></font></div>\n");
  }
  print("<form id='device' method='POST' style='margin: 0;'>\n");
  print("Sensor device hostname: ".$_SESSION['device']." <select name='device' onchange='dev_change()'>\n");
  print("<option>Select...</option>\n");
  print("<option value='Clamps'>Clamps</option>\n");
  print("<option value='Joey'>Joey</option>\n");
  print("<option value='Donbot'>Donbot</option>\n");
  print("</select></form>\t\n");
  print("<button id='change'>Change the date format</button>\n");
  print("</td><td>\n");
  print("<div id='chart_divTemp' style='width: 140px;'></div>\n");
  print("</td></tr></table>\n");
  print("<style>div.google-visualization-tooltip { ; }</style>\n");
  print("<div id='curve_chart' style='width: 1000px; height: 600px'></div></div>\n");
  print("<div class='tooltip'><a href='mailto:mail@hugo.ro?subject=Thermostat IoT'>&copy;2018</a>\n");
  print("<span class='tooltiptext'>by Hugo (and others)</span></div>\n");
  print("<script type='text/javascript'>\n");
  print("function dev_change() {\n");
  print("  document.getElementById('device').submit();\n");
  print("}\n");
  //print("</script>\n");
  //print("<script type='text/javascript'>\n");
  print("(function()\n");
  print("  { var Moment = moment.unix(".readDataFile()[1]."/1000).format('dddd, MMMM Do, YYYY HH:mm:ss');\n");
  print("    var eDisplayMoment = document.getElementById('displayMoment');\n");
  print("    eDisplayMoment.innerHTML = Moment;\n");
  print("  }\n");
  print(")();\n");
  print("</script>\n");
  print("</body></html>\n");
