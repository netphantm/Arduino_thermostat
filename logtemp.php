<?php

if(!empty($_GET["status"]) && !empty($_GET["temperature"]) && !empty($_GET["hostname"])) {
  $epoch = array_sum( explode( ' ' , microtime() ) );
  $epochTime = ($epoch * 1000);
  $niceTime = (new DateTime("$epochTime[0]"))->format('Y-m-d H:i:s');
  $interval = isset($_GET["interval"]) ? $_GET["interval"] : 120000;
  $temp_dev = isset($_GET["temp_dev"]) ? $_GET["temp_dev"] : 0;
  $csvData = array($_GET["status"],$_GET["temperature"],$_GET["temp_min"],$_GET["temp_max"],$epochTime,$_GET["heater"],$_GET["manual"],$interval,$temp_dev);
  print($epochTime.", ".$niceTime);

  $fcsv = fopen("/var/www/temp/temp-log-".$_GET["hostname"].".csv","a"); 
  if($fcsv) {
     fputcsv($fcsv,$csvData);
     fclose($fcsv);
  } else {
    error_log("[/var/www/temp/logtemp.php] - Something went wrong, could not append to file /var/www/temp/temp-log-".$_GET["hostname"].".csv");
  }
}
?>
