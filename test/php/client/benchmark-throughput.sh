# usage: ./benchmark.sh flamegraph_absolute_path profiler_absolute_path client_absolute_path clientsNr1 clientsNr2 ...
# # Example: ./benchmark.sh /path/to/flamegraph /path/to/profiler /path/to/client 1 2 4 8 16 32 64 128 256 512 1024
# It is expected that the flamegraph used is https://github.com/brendangregg/Flamegraph
if [ "$#" -lt 4 ]; then
    echo "Usage: $0 flamegraph_absolute_path profiler_absolute_path client_absolute_path msgNumber1 msgNumber2 ..."
    exit 1
fi

runThroughput() {
    writeToFile=$1
    clientPath=$2
    msgNumber=$3
    qType=$4

    echo "Running $msgNumber messages with client path: $clientPath"

    php $clientPath/throughput.php ${writeToFile} ${msgNumber} ${qType} > /dev/null
}

runBenchmark() {
    # generateLatexTableFromSVGsALLOC benchmark_results/1755695703 service_alloc_sampling benchmark_results/1755695703_alloc_sampling_table alloc
    # exit 1
    writeToFile=$1
    shift
    qType=$1
    shift
    clientAbsolutePath=$1
    shift
    # timestamp=$1
    # shift

    timestamp=$(date +%s)

    if [ $writeToFile == "true" ]; then
        if [ ! -d "benchmark_results/throughput/${timestamp}" ]; then
            mkdir -p benchmark_results/throughput/${timestamp}
        fi

        echo "messages_number,throughput,minLatency,maxLatency,avgLatencuy" > ./benchmark_results/throughput/${timestamp}/throughput_benchmark.csv
    fi

    for msgNumber in "$@"; do
        # servicePID=$(pgrep -f 'java -cp')

        # ${profilerPath}/build/bin/asprof start -e cpu,alloc,nativemem,lock -o jfr -f ./${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr ${servicePID}

        if [ $writeToFile == "true" ]; then
            if [ ! -d "benchmark_results/throughput/${msgNumber}" ]; then
                mkdir -p benchmark_results/throughput/${msgNumber}
            fi
        fi

        runThroughput $writeToFile $clientAbsolutePath $msgNumber $qType

        wait

        # cat benchmark_results/throughput/${msgNumber}/* | wc -l
        if [ $writeToFile == "true" ]; then
            cat benchmark_results/throughput/${msgNumber}/* >> ./benchmark_results/throughput/${timestamp}/throughput_benchmark.csv
            rm -rf benchmark_results/throughput/${msgNumber}
        fi

        echo "All $msgNumber messages sent."
    done

    # generateLatexTableFromSVGsCPU benchmark_results/${timestamp} service_cpu_sampling benchmark_results/${timestamp}_cpu_sampling_table cpu
    # generateLatexTableFromSVGsALLOC benchmark_results/${timestamp} service_alloc_sampling benchmark_results/${timestamp}_alloc_sampling_table alloc

    if [ $writeToFile == "true" ]; then
        source venv/bin/activate
        # python3 create_plots.py ./benchmark_results/throughput/${timestamp} benchmark_results/${timestamp}_cpu_sampling_table benchmark_results/${timestamp}_alloc_sampling_table ./benchmark_results/throughput/${timestamp}/throughput_benchmark.csv
        deactivate
    fi

    # mv benchmark_results/${timestamp}_cpu_sampling_table benchmark_results/${timestamp}/cpu_sampling_table
    # mv benchmark_results/${timestamp}_alloc_sampling_table benchmark_results/${timestamp}/alloc_sampling_table

    echo "Benchmark completed."
}

runBenchmark $@

# docker exec -it --user user -e TERM=xterm-256color -w /home/user docker-dsp-library-1 bash

# git clone https://github.com/brendangregg/FlameGraph.git
# git clone https://github.com/async-profiler/async-profiler.git
# ./benchmark.sh /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 500 750 1000 1500 2000 2500 3250 4000 5000 6000
# ./benchmark.sh /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32
# ./benchmark.sh /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024
