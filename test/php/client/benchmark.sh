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

    running=0
    maxRunning=256
    currentBatchNr=1

    for i in $(seq 1 $clientsNr); do
        if (( (i - 1) % maxRunning == 0 )); then
            echo "Starting batch" ${currentBatchNr}
            ((currentBatchNr++))
        fi
        php $clientPath/main.php > /dev/null &
        ((running++))

        if (( running >= maxRunning )); then
            sleep 1
            running=0
        fi
    done
}

parseProfilingResult() {
    profilerPath=$1
    flamegraphPath=$2

    jfrFile=$3
    type=$4
    collapsedFile=$5

    outPath=$6

    if [ ! -d ${outPath} ]; then
        mkdir ${outPath}
    fi

    flamegraphAttributes="--width 3000 --minWidth 5 --fontsize 14"

    ${profilerPath}/build/bin/jfrconv ${type} -t ${jfrFile}.jfr -o collapsed # --title thread_${cpuFlamegraphTitle}
    mv ${jfrFile}.collapsed ./${collapsedFile}.collapsed
    ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${collapsedFile} ./${collapsedFile}.collapsed > ${outPath}/thread_${collapsedFile}.svg
    rm ./${collapsedFile}.collapsed

    ${profilerPath}/build/bin/jfrconv ${type} ${jfrFile}.jfr -o collapsed # --title ${cpuFlamegraphTitle}
    mv ${jfrFile}.collapsed ./${collapsedFile}.collapsed
    ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title ${collapsedFile} ./${collapsedFile}.collapsed > ${outPath}/${collapsedFile}.svg
    rm ./${collapsedFile}.collapsed
}

convertSVGToPDF() {
    svgFile=$1
    outPath=$2

    if [ ! -d ${outPath} ]; then
        echo "Out path "${outPath}" does not exist!"
        return 1
    fi

    cd ${outPath}
    inkscape ./thread_${svgFile}.svg --export-pdf=thread_${svgFile}.pdf 2>/dev/null
    rm ./thread_${svgFile}.svg
    inkscape ./${svgFile}.svg --export-pdf=${svgFile}.pdf 2>/dev/null
    rm ./${svgFile}.svg
    cd -
}

runBenchmark() {
    flamegraphPath=$1
    shift
    profilerPath=$1
    shift
    clientAbsolutePath=$1
    shift

    for clientsNr in "$@"; do
        cpuFlamegraphTitle="service_cpu_benchmark_${clientsNr}_clients_sampling"
        allocFlamegraphTitle="service_alloc_benchmark_${clientsNr}_clients_sampling"
        cacheMissFlamegraphTitle="service_cache_miss_benchmark_${clientsNr}_clients_sampling"
        cpuAllocFlamegraphTitle="service_cpu_alloc_benchmark_${clientsNr}_clients_sampling"
        nativeMemFlamegraphTitle="service_nativemem_benchmark_${clientsNr}_clients_sampling"
        lockFlamegraphTitle="service_lock_benchmark_${clientsNr}_clients_sampling"
        cpuAllocNativeMemFlamegraphTitle="service_cpu_alloc_nativemem_benchmark_${clientsNr}_clients_sampling"
        cpuAllocNativeMemLockFlamegraphTitle="service_cpu_alloc_nativemem_lock_benchmark_${clientsNr}_clients_sampling"

        timestamp=$(date +%s)
        servicePID=$(pgrep -f 'java -cp')
        ${profilerPath}/build/bin/asprof start -e cpu,alloc,nativemem,lock -o jfr -f ./${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr ${servicePID}

        runClients $clientAbsolutePath $clientsNr

        wait

        ${profilerPath}/build/bin/asprof stop -o jfr -f ./${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr ${servicePID}

        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --cpu ${timestamp}_${cpuFlamegraphTitle} ./benchmark_results/cpu
        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --alloc ${timestamp}_${allocFlamegraphTitle} ./benchmark_results/alloc
        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --nativemem ${timestamp}_${nativeMemFlamegraphTitle} ./benchmark_results/native_alloc
        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --lock ${timestamp}_${lockFlamegraphTitle} ./benchmark_results/lock

        rm ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr

        convertSVGToPDF ${timestamp}_${cpuFlamegraphTitle} ./benchmark_results/cpu
        convertSVGToPDF ${timestamp}_${allocFlamegraphTitle} ./benchmark_results/alloc
        convertSVGToPDF ${timestamp}_${nativeMemFlamegraphTitle} ./benchmark_results/native_alloc
        convertSVGToPDF ${timestamp}_${lockFlamegraphTitle} ./benchmark_results/lock

        echo "All $clientsNr clients have finished."
    done
    echo "Benchmark completed."
}

runBenchmark $@

# docker exec -it --user user -e TERM=xterm-256color -w /home/user docker-dsp-library-1 bash

# git clone https://github.com/brendangregg/FlameGraph.git
# git clone https://github.com/async-profiler/async-profiler.git
# ./benchmark.sh /home/user/FlameGraph /home/user/async-profiler $(pwd) 500 750 1000 1500 2000 2500 3250 4000
