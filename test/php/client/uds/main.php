<?php

$socketPath = "/tmp/xslt.sock";
$timeout = 1;

$fp = stream_socket_client("unix://$socketPath", $errno, $errstr, $timeout, STREAM_CLIENT_CONNECT);

if (!$fp) {
    throw new RuntimeException("UDS connect failed: $errstr ($errno)");
}

stream_set_timeout($fp, $timeout);

$msg = "Hello from PHP UDS client";
fwrite($fp, $msg);

fclose($fp);
