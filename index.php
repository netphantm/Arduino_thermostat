<?php
  if (session_status() == PHP_SESSION_NONE  || session_id() == '') {
    session_start();
  }

  if (!empty($_POST['device'])) {
    $_SESSION['device'] = $_POST['device'];
  } else {
    $_SESSION['device'] = "Clamps";
  }

  header("refresh:300; url=/", FALSE, 307);

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
        $filename = 'temp-log-10.150.0.96.csv';
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
        //$uptime = $line[0];
        $state = $line[0];
        $temp = $line[1];
        $temp_min = $line[2];
        $temp_max = $line[3];
        $date = $line[4];
        $heater = $line[5];
        $manual = $line[6];

        //$retStr = $retStr."\n      [new Date(".$date."), ".$temp.", '<table><tr><td>".options.hAxis."</td></tr><tr><td>Temp: ".$temp."</td></tr><tr><td>State: ".$state."</td></tr></table>'], ";
        $retStr = $retStr."\n      [new Date(".$date."), '".$state."', ".$temp."], ";
      }
    }
    fclose($fp);
    return array ($retStr, $date, $state, $temp_min, $temp_max, $temp, $heater, $manual);
  }
?>

<html><head>
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
<meta http-equiv='refresh' content='60'>
<script src='moment.min.js' type='text/javascript'></script>

<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>
<script type='text/javascript'>
  google.charts.load('current', {'packages':['corechart','gauge']});
  google.charts.setOnLoadCallback(drawTempChart);
  google.charts.setOnLoadCallback(drawChart);

  function drawTempChart() {
    var data = google.visualization.arrayToDataTable([
      ['Label', 'Value'],
      ['Temp °C',
<?php
      print(readDataFile()[5]);
?>
    ], ]);

    var options = {
      width: 250, height: 150,
      min: 0, max: 40,
<?php
print("      greenFrom: 0, greenTo: ".readDataFile()[3].",\n");
print("      yellowFrom: ".readDataFile()[3].", yellowTo: ".readDataFile()[4].",\n");
print("      redFrom: ".readDataFile()[4].", redTo: 40,\n");
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
      {label: 'Temperature', type: 'number', role: 'data'}],

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
      curveType: 'function',
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
      options.hAxis.format === 'dd MMM, HH:mm' ?
      options.hAxis.format = 'd/M-HH:mm' :
      options.hAxis.format = 'dd MMM, HH:mm';
      chart.draw(data, options); 
    };
  }
</script>

<link rel='shortcut icon' href='https://www.hugo.ro/favicon.ico' />
<?php
  print("<title>".$_SESSION['device']." - Thermostat IoT</title>");
?>
</head><body>
<div class='content'>
<div align='center'><h2>ESP8266/WeMos D1 Mini Pro - DS18B20
<?php
  print("<br>".$_SESSION['device']." - IoT Thermostat</h2></div>\n");
?>
<div align='center'><table style='width:950px;'><tr><td>
</td></tr></table>
<table style='width:950px;'><tr><td>
<div>Last Readings: <span id="displayMoment"></span></div>
<?php
  print("<div>Temperature: <b>".readDataFile()[5]." °C</b></div>\n");
  print("<div>Hysteresis: <b>".readDataFile()[3]." °C > ".readDataFile()[4]." °C</b></div>\n");
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
  print("<form id='device' method='POST'>\n");
  print("Sensor device hostname: ".$_SESSION['device']." <select name='device' onchange='dev_change()'>\n");
?>
<option>Select...</option>
<option value='Clamps'>Clamps</option>
<option value='Joey'>Joey</option>
<option value='Donbot'>Donbot</option>
</select></form>
<form id='settings' method='POST' action='/settings.php'>
<?php
  print("<button id='settings' name='device' value=".$_SESSION['device'].">Settings</button></form>\n");
?>
</td><td>
<div id='chart_divTemp' style='width: 140px;'></div>
</td></tr></table>
<style>div.google-visualization-tooltip { ; }</style>
<div id='curve_chart' style='width: 1000px; height: 600px'></div></div>
<button id='change'>Change the date format</button>
<div class='tooltip'><a href='mailto:mail@hugo.ro?subject=Thermostat IoT'>&copy;2018</a>
<span class='tooltiptext'>by Hugo (and others)</span></div>
<script type='text/javascript'>
  function dev_change() {
    document.getElementById('device').submit()
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
</body></html>
