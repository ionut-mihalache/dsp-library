<?php
    $ffi = FFI::cdef("
        void hello();
        void increment();

        void connect();
    ",
    "/workspaces/ionut/Dev Containers/dsp-library/libdsp.so");

    // $ffi->hello();

    $ffi->connect();

    $i = 0;
    while ($i < 10) {
        $ffi->increment();
        sleep(3);
        $i++;
    }
?>
