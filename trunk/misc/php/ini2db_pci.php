<?php
	$filename = "./db/pci_dev.ini";
	$vendors_table = "pci_vendors_db";
	$devices_table = "pci_devices_db";
	
	// Data for mysql connection
	$mysql_host = "";  // Host
	$mysql_user = "";        // User Name
	$mysql_pass = ""; // Password
	$mysql_db   = "";  // DB Name
	
	$mysql = mysql_pconnect($mysql_host, $mysql_user, $mysql_pass);
	mysql_select_db($mysql_db, $mysql);

	$file = fopen($filename, "r");

	while (!feof($file))
	{
		$buffer = fgets($file, 1024);

		if ($buffer[0] == '[' || strlen($buffer) < 6)
			continue;

		$str = substr($buffer, 4, 1);
		if ($str == '=')
		{
			$venid = substr($buffer, 0, 4);
			$name = substr($buffer, 5);

			echo $venid." ::: ".$name."<br>";
			
			$query = "INSERT INTO ".$vendors_table." (venid, name) VALUES('".$venid."', '".$name."')";
			mysql_query($query, $mysql);
		}
		else if ($str == '-')
		{
			$devid = substr($buffer, 0, 9);
			$name = substr($buffer, 10);

			echo $devid." ::: ".$name."<br>";

			$query = "INSERT INTO ".$devices_table." (devid, name) VALUES('".$devid."', '".$name."')";
			mysql_query($query, $mysql);
		}
	}

	fclose($file);
	mysql_close($mysql);
?>