<?php
    $ffi = FFI::cdef("
        void hello();
        void increment();

        void dspConnect(const char* p_ServiceStrId);
    ",
    "/home/user/dsp-library/libdsp.so");


    // $ffi->hello();

    $ffi->dspConnect("xslt-transformation");

    // $i = 0;
    // while ($i < 10) {
    //     $ffi->increment();
    //     sleep(3);
    //     $i++;
    // }
?>
