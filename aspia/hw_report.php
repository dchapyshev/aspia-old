<?php
	// Data for mysql connection
	$mysql_host = "";  // Host
	$mysql_user = "";        // User Name
	$mysql_pass = ""; // Password
	$mysql_db   = "";  // DB Name

	$ip = getenv("REMOTE_ADDR");
	$date = date("d.m.Y H:i:s");

	$filename = "./reports/".md5($ip.$date).".ini";

	copy($_FILES['data']['tmp_name'], $filename);

	$mysql = mysql_connect($mysql_host, $mysql_user, $mysql_pass);
	mysql_select_db($mysql_db, $mysql);
	
	$file = fopen($filename, "r");

	while (!feof($file))
	{
		$buffer = fgets($file, 1024);

		$buffer = str_replace(array("'",'"','<','>','`','*'), "", $buffer);

		list($type, $devid, $name) = explode(":", $buffer);
		
		$devid = strtolower($devid);

		if (strcmp($type, "MON") == 0)
			$table = "mon_devices_db";
		else if (strcmp($type, "USB") == 0)
			$table = "usb_devices_db";
		else if (strcmp($type, "PCI") == 0)
			$table = "pci_devices_db";
		else continue;

		$query = "SELECT * FROM ".$table." WHERE devid='".$devid."'";
		$result = mysql_query($query, $mysql);
		$count = mysql_num_rows($result, $mysql);
		if ($count > 0) continue;

		$query = "INSERT INTO ".$table." (devid, name) VALUES('".$devid."', '".$name."')";
		mysql_query($query, $mysql);
	}

	fclose($file);
	mysql_close($mysql);

	//unlink($filename);
?>