<?php

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

$socketPath = "/tmp/xslt.sock";
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

$payload = str_pad($payload, 65548, "\0");

$benchmark["call"] = measureFnExec(function () use ($fp, $payload) {
    $written = 0;

    while ($written < 65548) {
        $partialWritten = fwrite($fp, substr($payload, $written));
        $written += $partialWritten;
    }
});

$benchmark["return"] = measureFnExec(function () use ($fp) {
    $read = 0;
    $response = '';

    while ($read < 65548) {
        $partialResponse = fread($fp, 65548 - $read);
        $response .= $partialResponse;

        $read += strlen($partialResponse);
    }

    echo "Received reply: $response\n";
});

$benchmark["disconnect"] = measureFnExec(function () use ($fp) {
    fclose($fp);
});

$uniqueId = uniqid("", true);

$file = fopen("benchmark_results/clients/" . $argv[1] . "/client_benchmark_" . $uniqueId . ".csv", 'a');
fputcsv($file, [
    $argv[1],
    $benchmark["connect"],
    $benchmark["call"],
    $benchmark["return"],
    $benchmark["disconnect"]
]);
fclose($file);
