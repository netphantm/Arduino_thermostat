<?php

if(!empty($_GET["status"]) && !empty($_GET["temperature"]) && !empty($_GET["IP"])) {
  $epoch = array_sum( explode( ' ' , microtime() ) );
  $epochTime = ($epoch * 1000);
  //$niceTime = (new DateTime("$epochTime[0]"))->format('Y-m-d H:i:s');
  $csvData = array($_GET["status"],$_GET["temperature"],$_GET["temp_min"],$_GET["temp_max"],$epochTime,$_GET["heater"],$_GET["manual"]);
  print($epochTime);

  $fcsv = fopen("/var/www/temp/temp-log-".$_GET["IP"].".csv","a"); 
  if($fcsv) {
     fputcsv($fcsv,$csvData);
     fclose($fcsv);
  } else {
    error_log("[/var/www/temp/logtemp.php] - Something went wrong, could not append to file /var/www/temp/temp-log-".$_GET["IP"].".csv");
  }
}
?>
