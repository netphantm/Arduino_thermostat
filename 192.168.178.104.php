<?php

// renders the temp from the DS18B20 sensor log from 'Clamps'
// using a nice google gauge and an even nicer google chart

// needed for DEBUG
function pr($var) {
  print '<pre>';
  print_r($var);
  print '</pre>';
}

function readDataFile() {
  $filename = 'temp-log-192.168.178.104.csv';
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
      $date = $line[3];

      //$retStr = $retStr."\n      [new Date(".$date."), ".$temp.", '<table><tr><td>".options.hAxis."</td></tr><tr><td>Temp: ".$temp."</td></tr><tr><td>State: ".$state."</td></tr></table>'], ";
      $retStr = $retStr."\n      [new Date(".$date."), '".$state."', ".$temp."], ";
    }
  }
  fclose($fp);
  return array ($retStr,$date,$state,$temp);
}

////////////////////////////////////////////////////////////////
//      creates the HTML string to be sent to the client      //
////////////////////////////////////////////////////////////////

  print("<html><head>\n");
  #print(pr(readDataFile()));
  print("<style>\n");
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
  print("<meta http-equiv=\"refresh\" content=\"60;url=/192.168.178.104.php\"> \n");
  print("<script src=\"moment.min.js\" type=\"text/javascript\"></script>\n");

  ///////////// google charts script
  // setup the google gauge options here
  print("<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script> \n");
  print("<script type=\"text/javascript\"> \n");
  print("  google.charts.load('current', {'packages':['corechart','gauge']}); \n");
  print("  google.charts.setOnLoadCallback(drawTempChart); \n");
  print("  google.charts.setOnLoadCallback(drawChart); \n ");

  // draw temp gaug)e
  print("  function drawTempChart() { \n");
  print("    var data = google.visualization.arrayToDataTable([ \n");
  print("      ['Label', 'Value'], \n");
  print("      ['Temp °C', ");
  print(readDataFile()[3]);
  print("], ]); \n");

  // setup the google gauge options here
  print("    var options = { \n");
  print("      width: 250, height: 150, \n");
  print("      min: 0, max: 40, \n");
  print("      greenFrom: 0, greenTo: 8, \n");
  print("      yellowFrom: 8, yellowTo: 12, \n");
  print("      redFrom: 12, redTo: 40, \n");
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
  print("<link rel=\"shortcut icon\" href=\"https://www.hugo.ro/favicon.ico\"/>\n");
  print("<title>'Clamps' Thermostat IoT</title>\n");
  print("<style>\n.content { background-color: lightcyan; width: 1000px; margin: auto; }</style>\n");
  print("</head><body>\n");

  print("\n<div class=\"content\">\n");
  print("<div align=\"center\"><h1>ESP8266-12E DS18B20 - Thermostat IoT</h1></div>\n");
  print("<div align=\"center\" style=\"color:red\">".$webMessage."</div>\n");
  print("<div align=\"center\"><table style=\"width:950px;\"><tr><td>");
  print("</td></tr></table>\n");
  print("<table style=\"width:950px;\"><tr><td>\n");
  print("<div>Last Readings: ");
  print("<span id=\"displayMoment\"></span>");
  print("</div><br>\n");
  print("<div>Temperature: ".readDataFile()[3]." °C</div><br>\n");
  if (readDataFile()[2] == "OFF") {
    print("<div>Relais is: <font style=\"color:red\"><b>OFF</b></font></div><br>\n");
  } else {
    print("<div>Relais is: <font style=\"color:red\"><b>ON</b></font></div><br>\n");
  }
  print("</td><td>\n");
  print("<div id=\"chart_divTemp\" style=\"width: 250px;\"></div>\n");
  print("</td></tr></table>\n");
  print("<button id=\"change\">Change the date format</button>\n");
  print("<style>div.google-visualization-tooltip { ; }</style>\n");
  print("<div id=\"curve_chart\" style=\"width: 1000px; height: 600px\"></div></div>\n");
  print("<div class=\"tooltip\"><a href=\"mailto:mail@hugo.ro?subject='Clamps' Thermostat IoT\">&copy;2018</a>\n");
  print("<span class=\"tooltiptext\">by Hugo (and others)</span></div>\n");
  print("<script type=\"text/javascript\">\n");
  print("(function()\n");
  print("  { var Moment = moment.unix(".readDataFile()[1]."/1000).format('dddd, MMMM Do, YYYY HH:mm:ss');\n");
  print("    var eDisplayMoment = document.getElementById('displayMoment');\n");
  print("    eDisplayMoment.innerHTML = Moment;\n");
  print("  }\n");
  print(")();\n");
  print("</script>\n");
  print("</body></html>\n");
