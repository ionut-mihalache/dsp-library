# usage: ./benchmark.sh flamegraph_absolute_path profiler_absolute_path client_absolute_path clientsNr1 clientsNr2 ...
# # Example: ./benchmark.sh /path/to/flamegraph /path/to/profiler /path/to/client 1 2 4 8 16 32 64 128 256 512 1024
# It is expected that the flamegraph used is https://github.com/brendangregg/Flamegraph
if [ "$#" -lt 4 ]; then
    echo "Usage: $0 flamegraph_absolute_path profiler_absolute_path client_absolute_path clientsNr1 clientsNr2 ..."
    exit 1
fi

runClients() {
    clientPath=$1
    clientsNr=$2
    echo "Running $clientsNr clients with client path: $clientPath"
    for i in $(seq 1 $clientsNr); do
        php $clientPath/main.php > /dev/null &
    done
}

runBenchmark() {
    flamegraphPath=$1
    shift
    profilerPath=$1
    shift
    clientAbsolutePath=$1
    shift

    flamegraphAttributes="--width 3000 --minWidth=5 --fontsize 14"

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

        ${profilerPath}/build/bin/jfrconv --cpu -t ${timestamp}_${cpuAllocFlamegraphTitle}.jfr -o collapsed # --title thread_${cpuFlamegraphTitle}
        # mv ${timestamp}_${cpuAllocFlamegraphTitle}.html ./cpu/thread_${timestamp}_${cpuFlamegraphTitle}.html
        mv ${timestamp}_${cpuAllocFlamegraphTitle}.collapsed ./${timestamp}_${cpuFlamegraphTitle}.collapsed
        ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${cpuFlamegraphTitle} ./${timestamp}_${cpuFlamegraphTitle}.collapsed > ./cpu/thread_${timestamp}_${cpuFlamegraphTitle}.svg
        rm ./${timestamp}_${cpuFlamegraphTitle}.collapsed

        ${profilerPath}/build/bin/jfrconv --alloc -t ${timestamp}_${cpuAllocFlamegraphTitle}.jfr -o collapsed #--title thread_${allocFlamegraphTitle}
        # mv ${timestamp}_${cpuAllocFlamegraphTitle}.html ./alloc/thread_${timestamp}_${allocFlamegraphTitle}.html
        mv ${timestamp}_${cpuAllocFlamegraphTitle}.collapsed ./${timestamp}_${allocFlamegraphTitle}.collapsed
        ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${allocFlamegraphTitle} ./${timestamp}_${allocFlamegraphTitle}.collapsed > ./alloc/thread_${timestamp}_${allocFlamegraphTitle}.svg
        rm ./${timestamp}_${allocFlamegraphTitle}.collapsed

        ${profilerPath}/build/bin/jfrconv --cpu ${timestamp}_${cpuAllocFlamegraphTitle}.jfr -o collapsed # --title ${cpuFlamegraphTitle}
        # mv ${timestamp}_${cpuAllocFlamegraphTitle}.html ./cpu/${timestamp}_${cpuFlamegraphTitle}.html
        mv ${timestamp}_${cpuAllocFlamegraphTitle}.collapsed ./${timestamp}_${cpuFlamegraphTitle}.collapsed
        ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title ${cpuFlamegraphTitle} ./${timestamp}_${cpuFlamegraphTitle}.collapsed > ./cpu/${timestamp}_${cpuFlamegraphTitle}.svg
        rm ./${timestamp}_${cpuFlamegraphTitle}.collapsed

        ${profilerPath}/build/bin/jfrconv --alloc ${timestamp}_${cpuAllocFlamegraphTitle}.jfr -o collapsed #--title ${allocFlamegraphTitle}
        # mv ${timestamp}_${cpuAllocFlamegraphTitle}.html ./alloc/${timestamp}_${allocFlamegraphTitle}.html
        mv ${timestamp}_${cpuAllocFlamegraphTitle}.collapsed ./${timestamp}_${allocFlamegraphTitle}.collapsed
        ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title ${allocFlamegraphTitle} ./${timestamp}_${allocFlamegraphTitle}.collapsed > ./alloc/${timestamp}_${allocFlamegraphTitle}.svg
        rm ./${timestamp}_${allocFlamegraphTitle}.collapsed

        rm ${timestamp}_${cpuAllocFlamegraphTitle}.jfr

        cd ./cpu
        inkscape ./thread_${timestamp}_${cpuFlamegraphTitle}.svg --export-pdf=thread_${timestamp}_${cpuFlamegraphTitle}.pdf 2>/dev/null
        rm ./thread_${timestamp}_${cpuFlamegraphTitle}.svg
        inkscape ./${timestamp}_${cpuFlamegraphTitle}.svg --export-pdf=${timestamp}_${cpuFlamegraphTitle}.pdf 2>/dev/null
        rm ./${timestamp}_${cpuFlamegraphTitle}.svg
        cd -

        cd ./alloc
        inkscape ./thread_${timestamp}_${allocFlamegraphTitle}.svg --export-pdf=thread_${timestamp}_${allocFlamegraphTitle}.pdf 2>/dev/null
        rm ./thread_${timestamp}_${allocFlamegraphTitle}.svg
        inkscape ./${timestamp}_${allocFlamegraphTitle}.svg --export-pdf=${timestamp}_${allocFlamegraphTitle}.pdf 2>/dev/null
        rm ./${timestamp}_${allocFlamegraphTitle}.svg
        cd -

        echo "All $clientsNr clients have finished."
    done
    echo "Benchmark completed."
}

runBenchmark $@
