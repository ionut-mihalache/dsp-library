# usage: ./benchmark.sh profiler_absolute_path client_absolute_path clientsNr1 clientsNr2 ...
# Example: ./benchmark.sh /path/to/profiler /path/to/client 1 2 3 4 5
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 profiler_absolute_path client_absolute_path clientsNr1 clientsNr2 ..."
    exit 1
fi

runClients() {
    clientPath=$1
    clientsNr=$2
    echo "Running $clientsNr clients with client path: $clientPath"
    for i in $(seq 1 $clientsNr); do
        php $clientPath/main.php &
    done
}

runBenchmark() {
    profilerPath=$1
    shift
    clientAbsolutePath=$1
    shift

    for clientsNr in "$@"; do
        samplingTime=$((clientsNr <= 128 ? (clientsNr < 128 ? 10 : clientsNr) : (clientsNr < 1024 ? 30 : 60)))
        samplingTime=$((samplingTime + 1))
        cpuFlamegraphTitle="service_cpu_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        allocFlamegraphTitle="service_alloc_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        cacheMissFlamegraphTitle="service_cache_miss_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        cpuAllocFlamegraphTitle="service_cpu_alloc_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"

        # ${profilerPath}/build/bin/asprof -e cpu -o flamegraph -d $samplingTime -f ./cpu/$(date +%s)_$cpuFlamegraphTitle.html --title $cpuFlamegraphTitle $(pgrep -f 'java -cp') &
        # ${profilerPath}/build/bin/asprof -e alloc -o flamegraph -d $samplingTime -f alloc/$(date +%s)_$allocFlamegraphTitle.html --title $allocFlamegraphTitle $(pgrep -f 'java -cp') &
        # ${profilerPath}/build/bin/asprof -e cache-misses -o flamegraph -d $samplingTime -f cache-misses/$(date +%s)_$cacheMissFlamegraphTitle.html --title $cacheMissFlamegraphTitle $(pgrep -f 'java -cp') &
        timestamp=$(date +%s)
        ${profilerPath}/build/bin/asprof -e cpu,alloc -o jfr -d $samplingTime -f ${timestamp}_${cpuAllocFlamegraphTitle}.jfr $(pgrep -f 'java -cp') &

        sleep 1 # Give some time for the profiler to start

        runClients $clientAbsolutePath $clientsNr

        wait

        ${profilerPath}/build/bin/jfrconv --cpu -t ${timestamp}_${cpuAllocFlamegraphTitle}.jfr --title thread_${cpuFlamegraphTitle}
        mv ${timestamp}_${cpuAllocFlamegraphTitle}.html ./cpu/thread_${timestamp}_${cpuFlamegraphTitle}.html
        ${profilerPath}/build/bin/jfrconv --alloc -t ${timestamp}_${cpuAllocFlamegraphTitle}.jfr --title thread_${allocFlamegraphTitle}
        mv ${timestamp}_${cpuAllocFlamegraphTitle}.html ./alloc/thread_${timestamp}_${allocFlamegraphTitle}.html

        ${profilerPath}/build/bin/jfrconv --cpu ${timestamp}_${cpuAllocFlamegraphTitle}.jfr --title ${cpuFlamegraphTitle}
        mv ${timestamp}_${cpuAllocFlamegraphTitle}.html ./cpu/${timestamp}_${cpuFlamegraphTitle}.html
        ${profilerPath}/build/bin/jfrconv --alloc ${timestamp}_${cpuAllocFlamegraphTitle}.jfr --title ${allocFlamegraphTitle}
        mv ${timestamp}_${cpuAllocFlamegraphTitle}.html ./alloc/${timestamp}_${allocFlamegraphTitle}.html

        rm ${timestamp}_${cpuAllocFlamegraphTitle}.jfr

        echo "All $clientsNr clients have finished."
    done
    echo "Benchmark completed."
}

runBenchmark $@
