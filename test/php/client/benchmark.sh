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

parseProfilingResult() {
    profilerPath=$1
    flamegraphPath=$2

    jfrFile=$3
    type=$4
    collapsedFile=$5

    outPath=$6

    flamegraphAttributes="--width 3000 --minWidth 5 --fontsize 14"

    ${profilerPath}/build/bin/jfrconv ${type} -t ${jfrFile}.jfr -o collapsed # --title thread_${cpuFlamegraphTitle}
    # mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.html ./cpu/thread_${timestamp}_${cpuFlamegraphTitle}.html
    mv ${jfrFile}.collapsed ./${collapsedFile}.collapsed
    ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${collapsedFile} ./${collapsedFile}.collapsed > ${outPath}/thread_${collapsedFile}.svg
    rm ./${collapsedFile}.collapsed

    ${profilerPath}/build/bin/jfrconv ${type} ${jfrFile}.jfr -o collapsed # --title ${cpuFlamegraphTitle}
    # mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.html ./cpu/${timestamp}_${cpuFlamegraphTitle}.html
    mv ${jfrFile}.collapsed ./${collapsedFile}.collapsed
    ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title ${collapsedFile} ./${collapsedFile}.collapsed > ${outPath}/${collapsedFile}.svg
    rm ./${collapsedFile}.collapsed
}

convertSVGToPDF() {
    svgFile=$1
    outPath=$2

    cd ${outPath}
    inkscape ./${svgFile}.svg --export-pdf=thread_${svgFile}.pdf 2>/dev/null
    # rm ./thread_${svgFile}.svg
    inkscape ./${svgFile}.svg --export-pdf=${svgFile}.pdf 2>/dev/null
    # rm ./${svgFile}.svg
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
        samplingTime=$((clientsNr <= 128 ? (clientsNr < 128 ? 10 : clientsNr) : (clientsNr < 1024 ? 30 : 60)))
        samplingTime=$((samplingTime + 1))
        cpuFlamegraphTitle="service_cpu_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        allocFlamegraphTitle="service_alloc_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        cacheMissFlamegraphTitle="service_cache_miss_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        cpuAllocFlamegraphTitle="service_cpu_alloc_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        nativeMemFlamegraphTitle="service_nativemem_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        lockFlamegraphTitle="service_lock_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        cpuAllocNativeMemFlamegraphTitle="service_cpu_alloc_nativemem_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"
        cpuAllocNativeMemLockFlamegraphTitle="service_cpu_alloc_nativemem_lock_benchmark_${clientsNr}_clients_sampling_${samplingTime}s"

        # ${profilerPath}/build/bin/asprof -e cpu -o flamegraph -d $samplingTime -f ./cpu/$(date +%s)_$cpuFlamegraphTitle.html --title $cpuFlamegraphTitle $(pgrep -f 'java -cp') &
        # ${profilerPath}/build/bin/asprof -e alloc -o flamegraph -d $samplingTime -f alloc/$(date +%s)_$allocFlamegraphTitle.html --title $allocFlamegraphTitle $(pgrep -f 'java -cp') &
        # ${profilerPath}/build/bin/asprof -e cache-misses -o flamegraph -d $samplingTime -f cache-misses/$(date +%s)_$cacheMissFlamegraphTitle.html --title $cacheMissFlamegraphTitle $(pgrep -f 'java -cp') &
        timestamp=$(date +%s)
        ${profilerPath}/build/bin/asprof -e cpu,alloc,nativemem,lock -o jfr -d $samplingTime -f ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr $(pgrep -f 'java -cp') &

        sleep 1 # Give some time for the profiler to start

        runClients $clientAbsolutePath $clientsNr

        wait

        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --cpu ${timestamp}_${cpuFlamegraphTitle} ./cpu
        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --alloc ${timestamp}_${allocFlamegraphTitle} ./alloc
        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --nativemem ${timestamp}_${nativeMemFlamegraphTitle} ./native_alloc
        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --lock ${timestamp}_${lockFlamegraphTitle} ./lock

        rm ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr

        convertSVGToPDF ${timestamp}_${cpuFlamegraphTitle} ./cpu
        convertSVGToPDF ${timestamp}_${allocFlamegraphTitle} ./alloc
        convertSVGToPDF ${timestamp}_${nativeMemFlamegraphTitle} ./native_alloc
        convertSVGToPDF ${timestamp}_${lockFlamegraphTitle} ./lock

        echo "All $clientsNr clients have finished."
    done
    echo "Benchmark completed."
}

runBenchmark $@

# ${profilerPath}/build/bin/jfrconv --cpu -t ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr -o collapsed # --title thread_${cpuFlamegraphTitle}
# # mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.html ./cpu/thread_${timestamp}_${cpuFlamegraphTitle}.html
# mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.collapsed ./${timestamp}_${cpuFlamegraphTitle}.collapsed
# ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${cpuFlamegraphTitle} ./${timestamp}_${cpuFlamegraphTitle}.collapsed > ./cpu/thread_${timestamp}_${cpuFlamegraphTitle}.svg
# rm ./${timestamp}_${cpuFlamegraphTitle}.collapsed

# ${profilerPath}/build/bin/jfrconv --alloc -t ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr -o collapsed #--title thread_${allocFlamegraphTitle}
# # mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.html ./alloc/thread_${timestamp}_${allocFlamegraphTitle}.html
# mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.collapsed ./${timestamp}_${allocFlamegraphTitle}.collapsed
# ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${allocFlamegraphTitle} ./${timestamp}_${allocFlamegraphTitle}.collapsed > ./alloc/thread_${timestamp}_${allocFlamegraphTitle}.svg
# rm ./${timestamp}_${allocFlamegraphTitle}.collapsed

# ${profilerPath}/build/bin/jfrconv --nativemem -t ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr -o collapsed #--title thread_${allocFlamegraphTitle}
# # mv ${timestamp}_${nativeMemFlamegraphTitle}.html ./alloc/thread_${timestamp}_${nativeMemFlamegraphTitle}.html
# mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.collapsed ./${timestamp}_${nativeMemFlamegraphTitle}.collapsed
# ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${nativeMemFlamegraphTitle} ./${timestamp}_${nativeMemFlamegraphTitle}.collapsed > ./native_alloc/thread_${timestamp}_${nativeMemFlamegraphTitle}.svg
# rm ./${timestamp}_${nativeMemFlamegraphTitle}.collapsed

# ${profilerPath}/build/bin/jfrconv --lock -t ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr -o collapsed #--title thread_${allocFlamegraphTitle}
# # mv ${timestamp}_${lockFlamegraphTitle}.html ./alloc/thread_${timestamp}_${lockFlamegraphTitle}.html
# mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.collapsed ./${timestamp}_${lockFlamegraphTitle}.collapsed
# ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${lockFlamegraphTitle} ./${timestamp}_${lockFlamegraphTitle}.collapsed > ./lock/thread_${timestamp}_${lockFlamegraphTitle}.svg
# rm ./${timestamp}_${lockFlamegraphTitle}.collapsed


# ${profilerPath}/build/bin/jfrconv --cpu ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr -o collapsed # --title ${cpuFlamegraphTitle}
# # mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.html ./cpu/${timestamp}_${cpuFlamegraphTitle}.html
# mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.collapsed ./${timestamp}_${cpuFlamegraphTitle}.collapsed
# ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title ${cpuFlamegraphTitle} ./${timestamp}_${cpuFlamegraphTitle}.collapsed > ./cpu/${timestamp}_${cpuFlamegraphTitle}.svg
# rm ./${timestamp}_${cpuFlamegraphTitle}.collapsed

# ${profilerPath}/build/bin/jfrconv --alloc ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr -o collapsed #--title ${allocFlamegraphTitle}
# # mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.html ./alloc/${timestamp}_${allocFlamegraphTitle}.html
# mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.collapsed ./${timestamp}_${allocFlamegraphTitle}.collapsed
# ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title ${allocFlamegraphTitle} ./${timestamp}_${allocFlamegraphTitle}.collapsed > ./alloc/${timestamp}_${allocFlamegraphTitle}.svg
# rm ./${timestamp}_${allocFlamegraphTitle}.collapsed

# ${profilerPath}/build/bin/jfrconv --nativemem ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr -o collapsed #--title ${allocFlamegraphTitle}
# # mv ${timestamp}_${nativeMemFlamegraphTitle}.html ./alloc/${timestamp}_${nativeMemFlamegraphTitle}.html
# mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.collapsed ./${timestamp}_${nativeMemFlamegraphTitle}.collapsed
# ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title ${nativeMemFlamegraphTitle} ./${timestamp}_${nativeMemFlamegraphTitle}.collapsed > ./native_alloc/${timestamp}_${nativeMemFlamegraphTitle}.svg
# rm ./${timestamp}_${nativeMemFlamegraphTitle}.collapsed

# ${profilerPath}/build/bin/jfrconv --lock ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr -o collapsed #--title thread_${allocFlamegraphTitle}
# # mv ${timestamp}_${lockFlamegraphTitle}.html ./alloc/thread_${timestamp}_${lockFlamegraphTitle}.html
# mv ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.collapsed ./${timestamp}_${lockFlamegraphTitle}.collapsed
# ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${lockFlamegraphTitle} ./${timestamp}_${lockFlamegraphTitle}.collapsed > ./lock/${timestamp}_${lockFlamegraphTitle}.svg
# rm ./${timestamp}_${lockFlamegraphTitle}.collapsed

# cd ./cpu
# inkscape ./thread_${timestamp}_${cpuFlamegraphTitle}.svg --export-pdf=thread_${timestamp}_${cpuFlamegraphTitle}.pdf 2>/dev/null
# # rm ./thread_${timestamp}_${cpuFlamegraphTitle}.svg
# inkscape ./${timestamp}_${cpuFlamegraphTitle}.svg --export-pdf=${timestamp}_${cpuFlamegraphTitle}.pdf 2>/dev/null
# # rm ./${timestamp}_${cpuFlamegraphTitle}.svg
# cd -

# cd ./alloc
# inkscape ./thread_${timestamp}_${allocFlamegraphTitle}.svg --export-pdf=thread_${timestamp}_${allocFlamegraphTitle}.pdf 2>/dev/null
# # rm ./thread_${timestamp}_${allocFlamegraphTitle}.svg
# inkscape ./${timestamp}_${allocFlamegraphTitle}.svg --export-pdf=${timestamp}_${allocFlamegraphTitle}.pdf 2>/dev/null
# # rm ./${timestamp}_${allocFlamegraphTitle}.svg
# cd -

# cd ./native_alloc
# inkscape ./thread_${timestamp}_${nativeMemFlamegraphTitle}.svg --export-pdf=thread_${timestamp}_${nativeMemFlamegraphTitle}.pdf 2>/dev/null
# # rm ./thread_${timestamp}_${nativeMemFlamegraphTitle}.svg
# inkscape ./${timestamp}_${nativeMemFlamegraphTitle}.svg --export-pdf=${timestamp}_${nativeMemFlamegraphTitle}.pdf 2>/dev/null
# # rm ./${timestamp}_${nativeMemFlamegraphTitle}.svg
# cd -

# cd ./lock
# inkscape ./thread_${timestamp}_${lockFlamegraphTitle}.svg --export-pdf=thread_${timestamp}_${lockFlamegraphTitle}.pdf 2>/dev/null
# # rm ./thread_${timestamp}_${lockFlamegraphTitle}.svg
# inkscape ./${timestamp}_${lockFlamegraphTitle}.svg --export-pdf=${timestamp}_${lockFlamegraphTitle}.pdf 2>/dev/null
# # rm ./${timestamp}_${lockFlamegraphTitle}.svg
# cd -
