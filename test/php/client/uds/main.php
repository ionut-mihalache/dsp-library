<?php

class Constants
{
    public const int SMB = 1 << 16;
    public const int EMB = 1 << 17;
    public const int QMB = 1 << 18;
    public const int HMB = 1 << 19;
    public const int MB = 1 << 20;
    public const int DMB = 1 << 21;
    public const int HGB = 1 << 29;
    public const int GB = 1 << 30;
};

$arg = $argv[3] ?? "SMB";

$payloadSize = match (strtoupper($arg)) {
    "SMB" => Constants::SMB,
    "EMB" => Constants::EMB,
    "QMB" => Constants::QMB,
    "HMB" => Constants::HMB,
    "MB"  => Constants::MB,
    "DMB" => Constants::DMB,
    "HGB" => Constants::HGB,
    "GB"  => Constants::GB,
    default => throw new InvalidArgumentException("Unknown payload type: $arg"),
};

function measureFnExec(callable $p_Fn): float
{
    $start = microtime(true);

    $p_Fn();

    $end = microtime(true);

    return ($end - $start) * 1_000_000;

    // $start = getrusage();
    // $p_Fn();
    // $end = getrusage();

    // $utime = ($end["ru_utime.tv_sec"] - $start["ru_utime.tv_sec"]) * 1_000_000;
    // $utime += ($end["ru_utime.tv_usec"] - $start["ru_utime.tv_usec"]);

    // $stime = ($end["ru_stime.tv_sec"] - $start["ru_stime.tv_sec"]) * 1_000_000;
    // $stime += ($end["ru_stime.tv_usec"] - $start["ru_stime.tv_usec"]);

    // return $utime + $stime; // CPU real microseconds
}

$benchmark = [];

$socketPath = "/tmp/xslt-uds.sock";
$timeout = 1;

$fp = false;
// Connect to the server
$benchmark["connect"] = measureFnExec(function () use ($socketPath, &$fp, $timeout) {
    $fp = stream_socket_client("unix://$socketPath", $errno, $errstr, $timeout, STREAM_CLIENT_CONNECT);

    if (!$fp) {
        throw new RuntimeException("UDS connect failed: $errstr ($errno)");
    }
});

stream_set_timeout($fp, $timeout);

// Send a message to the server
$iiaData = '<?xml version="1.0" encoding="UTF-8"?>

<iias-get-response xmlns="https://github.com/erasmus-without-paper/ewp-specs-api-iias/blob/stable-v7/endpoints/get-response.xsd" xmlns:c="https://github.com/erasmus-without-paper/ewp-specs-types-contact/tree/stable-v1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="&#10;        https://github.com/erasmus-without-paper/ewp-specs-api-iias/blob/stable-v7/endpoints/get-response.xsd&#10;        https://raw.githubusercontent.com/erasmus-without-paper/ewp-specs-api-iias/stable-v7/endpoints/get-response.xsd&#10;    "><iia><partner><hei-id>upb.ro</hei-id><iia-id>9e01bc4f-f607-43c1-881a-8329c3e26062</iia-id></partner><partner><hei-id>brussels.uni-foundation.eu</hei-id><iia-id>9e01bc4f-f607-43c1-881a-8329c3e26063</iia-id><signing-contact><c:contact-name>Test contact</c:contact-name><c:email>test.contact@test.ro</c:email></signing-contact></partner><in-effect>false</in-effect><cooperation-conditions><student-studies-mobility-spec><sending-hei-id>brussels.uni-foundation.eu</sending-hei-id><sending-ounit-id>5</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ef711143-b1ab-4412-8493-3476d5ff962f</receiving-ounit-id><receiving-first-academic-year-id>2026/2027</receiving-first-academic-year-id><receiving-last-academic-year-id>2029/2030</receiving-last-academic-year-id><mobilities-per-year>2</mobilities-per-year><recommended-language-skill><language>is</language><cefr-level>B2</cefr-level><subject-area><isced-f-code>0007</isced-f-code><isced-clarification>Engineering, manufacturing and construction</isced-clarification></subject-area></recommended-language-skill><subject-area><isced-f-code>0819</isced-f-code><isced-clarification>Agriculture, not elsewhere classified</isced-clarification></subject-area><blended>false</blended></student-studies-mobility-spec></cooperation-conditions></iia></iias-get-response>';

$len = strlen($iiaData);
$lenBytes = pack('N', $len);

$payload = $lenBytes . $iiaData;

$payload = str_pad($payload, $payloadSize, "\0");

$benchmark["call"] = measureFnExec(function () use ($fp, $payload, $payloadSize) {
    $written = 0;

    while ($written < $payloadSize) {
        $partialWritten = fwrite($fp, substr($payload, $written));

        if ($partialWritten === false || $partialWritten === 0) {
            usleep(50);
            continue;
        }

        $written += $partialWritten;
    }
});

$benchmark["return"] = measureFnExec(function () use ($fp, $payloadSize) {
    $read = 0;
    $response = '';

    while ($read < $payloadSize) {
        $partialResponse = fread($fp, $payloadSize - $read);
        $response .= $partialResponse;

        $read += strlen($partialResponse);
    }

    // $response = substr($response, 4, strlen($response) - 4);

    // echo "Received reply: $response\n";
});

$benchmark["disconnect"] = measureFnExec(function () use ($fp, $payloadSize) {
    fclose($fp);
});

$uniqueId = uniqid("", true);

$writeToFile = false;

if (isset($argv[1]) && $argv[1] == "true") {
    $writeToFile = true;
}

if ($writeToFile) {
    $file = fopen("benchmark_results/clients/" . $argv[2] . "/client_benchmark_" . $uniqueId . ".csv", 'a');
    fputcsv($file, [
        $argv[2],
        $benchmark["connect"],
        $benchmark["call"],
        $benchmark["return"],
        $benchmark["disconnect"]
    ]);
    fclose($file);
}
