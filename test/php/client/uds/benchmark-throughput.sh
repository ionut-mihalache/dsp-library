# usage: ./benchmark.sh flamegraph_absolute_path profiler_absolute_path client_absolute_path clientsNr1 clientsNr2 ...
# # Example: ./benchmark.sh /path/to/flamegraph /path/to/profiler /path/to/client 1 2 4 8 16 32 64 128 256 512 1024
# It is expected that the flamegraph used is https://github.com/brendangregg/Flamegraph
if [ "$#" -lt 4 ]; then
    echo "Usage: $0 flamegraph_absolute_path profiler_absolute_path client_absolute_path clientsNr1 clientsNr2 ..."
    exit 1
fi

runThroughput() {
    writeToFile=$1
    clientPath=$2
    msgNumber=$3
    qType=$4

    echo "Running $msgNumber messages with client path: $clientPath"

    php $clientPath/throughput.php $writeToFile ${msgNumber} ${qType} > /dev/null
}

runBenchmark() {
    writeToFile=$1
    shift
    payloadType=$1
    shift
    clientAbsolutePath=$1
    shift

    timestamp=$(date +%s)

    if [ $writeToFile == "true" ]; then
        if [ ! -d "benchmark_results/throughput/${timestamp}" ]; then
            mkdir -p benchmark_results/throughput/${timestamp}
        fi

        echo "messages_number,payload_size,throughput,minLatency,maxLatency,avgLatency" > ./benchmark_results/throughput/${timestamp}/throughput_benchmark.csv
    fi

    for msgNumber in "$@"; do
        if [ $writeToFile == "true" ]; then
            if [ ! -d "benchmark_results/throughput/${msgNumber}" ]; then
                mkdir -p benchmark_results/throughput/${msgNumber}
            fi
        fi

        runThroughput $writeToFile $clientAbsolutePath $msgNumber $payloadType

        wait

        # cat benchmark_results/throughput/${msgNumber}/* | wc -l
        if [ $writeToFile == "true" ]; then
            cat benchmark_results/throughput/${msgNumber}/* >> ./benchmark_results/throughput/${timestamp}/throughput_benchmark.csv
            rm -rf benchmark_results/throughput/${msgNumber}
        fi

        echo "All $msgNumber messages sent."
    done

    if [ $writeToFile == "true" ]; then
        source ../venv/bin/activate
        # python3 create_plots.py ./benchmark_results/throughput/${timestamp} benchmark_results/${timestamp}_cpu_sampling_table benchmark_results/${timestamp}_alloc_sampling_table ./benchmark_results/throughput/${timestamp}/client_benchmark.csv
        deactivate
    fi

    echo "Benchmark completed."
}

runBenchmark $@

# docker exec -it --user user -e TERM=xterm-256color -w /home/user docker-dsp-library-1 bash

# git clone https://github.com/brendangregg/FlameGraph.git
# git clone https://github.com/async-profiler/async-profiler.git
# ./benchmark.sh /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 500 750 1000 1500 2000 2500 3250 4000 5000 6000
