<?php
    $ip = getenv("REMOTE_ADDR");
    copy($_FILES['data']['filename'], $ip."_".basename($_FILES['data']['name']));
?>