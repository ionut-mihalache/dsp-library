runAQUAClientBenchmark() {
    qType=$1

    rm /dev/shm/*

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ubuntu/dsp-library

    cd /home/ubuntu/dsp-library/test/java/service/xslt-transformation
    make clean
    make
    make run PAYLOAD_TYPE=${qType} &
    aquaPid=$!
    sleep 2
    cd -

    for i in $(seq 1 9);
    do
        ./benchmark.sh false $qType /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024
    done

    ./benchmark.sh true $qType /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024

    kill $aquaPid
}

runAQUAThroughputBenchmark() {
    qType=$1
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ubuntu/dsp-library

    msgNumbers=(1000 4000 7000 10000 13000 16000 19000 22000)

    for i in $(seq 1 9);
    do
        rm /dev/shm/*

        cd /home/ubuntu/dsp-library/test/java/service/xslt-transformation
        make clean
        make
        make throughput-run PAYLOAD_TYPE=${qType} &
        aquaPid=$!
        sleep 2
        cd -

        ./benchmark-throughput.sh false $qType $(pwd) 1000 4000 7000 10000 13000 16000 19000 22000

        kill $aquaPid
    done

    rm /dev/shm/*

    cd /home/ubuntu/dsp-library/test/java/service/xslt-transformation
    make clean
    make
    make throughput-run PAYLOAD_TYPE=${qType} &
    aquaPid=$!
    sleep 2
    cd -

    ./benchmark-throughput.sh true $qType $(pwd) 1000 4000 7000 10000 13000 16000 19000 22000

    kill $aquaPid
}

runUDSClientBenchmark() {
    qType=$1

    cd /home/ubuntu/dsp-library/test/java/service/xslt-transformation/src/uds
    make clean
    make
    make run PAYLOAD_TYPE=${qType} &
    udsPid=$!
    sleep 2
    cd -

    cd ./uds

    for i in $(seq 1 9);
    do
        ./benchmark.sh false $qType /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024
    done

    ./benchmark.sh true $qType /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024

    cd -

    kill $udsPid
}

runZeroMQClientBenchmark() {
    qType=$1

    cd /home/ubuntu/dsp-library/test/java/service/xslt-transformation/src/zeromq
    make clean
    make
    make run PAYLOAD_TYPE=${qType} &
    zeromqPid=$!
    sleep 2
    cd -

    cd ./zeromq

    for i in $(seq 1 9);
    do
        ./benchmark.sh false $qType /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024
    done

    ./benchmark.sh true $qType /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024

    cd -

    kill $zeromqPid
}

runClientBenchmark() {
    runAQUAClientBenchmark HMB
    runUDSClientBenchmark HMB
    runZeroMQClientBenchmark HMB

    lastAQUAbenchmark=$(ls benchmark_results/clients/ | grep -o "[0-9]\+" | sort -rn | head -n 1)
    lastUDSbenchmark=$(ls uds/benchmark_results/clients/ | grep -o "[0-9]\+" | sort -rn | head -n 1)
    lastZMQbenchmark=$(ls zeromq/benchmark_results/clients/ | grep -o "[0-9]\+" | sort -rn | head -n 1)

    metrics2Path="metrics2_hmb"
    metrics3Path="metrics3_hmb"

    if [ ! -d ${metrics2Path} ]; then
        mkdir -p ${metrics2Path}
    fi
    if [ ! -d ${metrics3Path} ]; then
        mkdir -p ${metrics3Path}
    fi

    cp benchmark_results/clients/$lastAQUAbenchmark/client_benchmark.csv ${metrics2Path}/aqua_client_benchmark.csv
    cp uds/benchmark_results/clients/$lastUDSbenchmark/client_benchmark.csv ${metrics2Path}/uds_client_benchmark.csv

    cp benchmark_results/clients/$lastAQUAbenchmark/client_benchmark.csv ${metrics3Path}/aqua_client_benchmark.csv
    cp uds/benchmark_results/clients/$lastUDSbenchmark/client_benchmark.csv ${metrics3Path}/uds_client_benchmark.csv
    cp zeromq/benchmark_results/clients/$lastZMQbenchmark/client_benchmark.csv ${metrics3Path}/zeromq_client_benchmark.csv

    source venv/bin/activate
    python3 create_combined_plots.py ${metrics2Path} benchmark_results/clients/$lastAQUAbenchmark/client_benchmark.csv uds/benchmark_results/clients/$lastUDSbenchmark/client_benchmark.csv
    python3 create_combined_plots.py ${metrics3Path} benchmark_results/clients/$lastAQUAbenchmark/client_benchmark.csv uds/benchmark_results/clients/$lastUDSbenchmark/client_benchmark.csv zeromq/benchmark_results/clients/$lastZMQbenchmark/client_benchmark.csv
    deactivate
}

# qTypes=(SMB EMB QMB HMB MB DMB HGB GB)

# for qType in "${qTypes[@]}";
# do
#     runAQUAThroughputBenchmark $qType
# done

# runAQUAClientBenchmark SMB
# runAQUAThroughputBenchmark SMB

runClientBenchmark

# ./benchmark.sh /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) $(date +%s) 128 256 384 512 640 758 886 1024