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
        php $clientPath/main.php ${clientsNr} > /dev/null &
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
        mkdir -p ${outPath}
    fi

    flamegraphAttributes="--width 1920 --height 12 --minWidth 15 --fontsize 12 --flamechart"
    # flamegraphAttributes="--width 1920 --minWidth 5 --fontsize 14"

    # ${profilerPath}/build/bin/jfrconv ${type} -t ${jfrFile}.jfr -o collapsed # --title thread_${cpuFlamegraphTitle}
    # mv ${jfrFile}.collapsed ./thread_${collapsedFile}.collapsed
    # ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title thread_${collapsedFile} ./thread_${collapsedFile}.collapsed > ${outPath}/thread_${collapsedFile}.svg
    # cp ./thread_${collapsedFile}.collapsed ${outPath}/thread_${collapsedFile}.collapsed
    # rm ./thread_${collapsedFile}.collapsed

    ${profilerPath}/build/bin/jfrconv ${type} ${jfrFile}.jfr -o collapsed # --title ${cpuFlamegraphTitle}
    mv ${jfrFile}.collapsed ./${collapsedFile}.collapsed
    ${flamegraphPath}/flamegraph.pl ${flamegraphAttributes} --title ${collapsedFile} ./${collapsedFile}.collapsed > ${outPath}/${collapsedFile}.svg
    # cp ./${collapsedFile}.collapsed ${outPath}/${collapsedFile}.collapsed
    rm ./${collapsedFile}.collapsed
}

convertSVGToPDF() {
    svgFile=$1
    outPath=$2

    if [ ! -d ${outPath} ]; then
        echo "Out path "${outPath}" does not exist!"
        return 1
    fi

    inkscapeAttributes="--export-area-drawing --export-text-to-path=no"

    cd ${outPath}
    # inkscape ./thread_${svgFile}.svg --export-pdf=thread_${svgFile}.pdf --export-area-drawing --export-text-to-path=no 2>/dev/null
    # rm ./thread_${svgFile}.svg
    inkscape ./${svgFile}.svg --export-pdf=${svgFile}.pdf 2>/dev/null
    # rm ./${svgFile}.svg
    cd -
}

generateLatexTableFromSVGsCPU() {
    rootDir=$1
    shift
    svgFile=$1
    shift
    outFile=$1
    shift
    sampleDir=$1

    WIDTH=80

    PADDING=$(( (WIDTH - 10 - ${#sampleDir}) / 2 ))
    FILL=$(printf '%*s' "$PADDING" '' | tr ' ' '=')

    START_LINE="${FILL} START OF ${sampleDir^^} RESULTS ${FILL}"
    END_LINE="${FILL} END OF ${sampleDir^^} RESULTS ${FILL}"

    # If the length is odd, add one more '=' to reach 80 chars
    while [ ${#START_LINE} -lt $WIDTH ]; do START_LINE+="="; done
    while [ ${#END_LINE} -lt $WIDTH ]; do END_LINE+="="; done

    echo ${START_LINE}
    echo -e "\\\\begin{table}[htbp]" > ${outFile}
    echo -e "\t\\\\caption{AQUA runtime CPU sampling}" >> ${outFile}
    echo -e "\t\\\\centering" >> ${outFile}
    echo -e "\t\\\\begin{tabular}{ccccc}" >> ${outFile}
    echo -e "\t\t\\\\toprule" >> ${outFile}
    echo -e "\t\t\\# & \\\\% & \\\\% & \\\\% & \\\\% \\\\\\\\" >> ${outFile}
    echo -e "\t\tclients & connect & disconnect & receive & send \\\\\\\\" >> ${outFile}
    echo -e "\t\t\\\\midrule" >> ${outFile}
    for dir in $(ls ${rootDir} | sort -n); do
        connect=$(grep "ConnectThread" ${rootDir}/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
        disconnect=$(grep "DisconnectThread" ${rootDir}/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
        receive=$(grep "receiveCall" ${rootDir}/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
        send=$(grep "sendReturn" ${rootDir}/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')

        echo ${dir} ${connect} ${disconnect} ${receive} ${send}

        if [[ -n "$connect" ]]; then
            connect="${connect%\%}"
        else
            connect="< 1"
        fi

        if [[ -n "$disconnect" ]]; then
            disconnect="${disconnect%\%}"
        else
            disconnect="< 1"
        fi

        if [[ -n "$receive" ]]; then
            receive="${receive%\%}"
        else
            receive="< 1"
        fi

        if [[ -n "$send" ]]; then
            send="${send%\%}"
        else
            send="< 1"
        fi

        echo -e "\t\t${dir} & ${connect} & ${disconnect} & ${receive} & ${send} \\\\\\\\" >> ${outFile}
    done
    echo -e "\t\t\\\\bottomrule" >> ${outFile}
    echo -e "\t\\\\end{tabular}" >> ${outFile}
    echo -e "\t\\\\label{tab:cpu_sampling}" >> ${outFile}
    echo -e "\\\\end{table}" >> ${outFile}
    echo ${END_LINE}
}

generateLatexTableFromSVGsALLOC() {
    rootDir=$1
    shift
    svgFile=$1
    shift
    outFile=$1
    shift
    sampleDir=$1

    WIDTH=80

    PADDING=$(( (WIDTH - 10 - ${#sampleDir}) / 2 ))
    FILL=$(printf '%*s' "$PADDING" '' | tr ' ' '=')

    START_LINE="${FILL} START OF ${sampleDir^^} RESULTS ${FILL}"
    END_LINE="${FILL} END OF ${sampleDir^^} RESULTS ${FILL}"

    # If the length is odd, add one more '=' to reach 80 chars
    while [ ${#START_LINE} -lt $WIDTH ]; do START_LINE+="="; done
    while [ ${#END_LINE} -lt $WIDTH ]; do END_LINE+="="; done

    echo ${START_LINE}
    echo -e "\\\\begin{table}[htbp]" > ${outFile}
    echo -e "\t\\\\caption{AQUA runtime ALLOC sampling}" >> ${outFile}
    echo -e "\t\\\\centering" >> ${outFile}
    echo -e "\t\\\\begin{tabular}{cccc}" >> ${outFile}
    echo -e "\t\t\\\\toprule" >> ${outFile}
    echo -e "\t\t\\# & \\\\% & \\\\% & \\\\% \\\\\\\\" >> ${outFile}
    echo -e "\t\tclients & connect & receive & send \\\\\\\\" >> ${outFile}
    echo -e "\t\t\\\\midrule" >> ${outFile}
    for dir in $(ls ${rootDir} | sort -n); do
        connect=$(grep "ConnectThread" ${rootDir}/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
        receive=$(grep "call_package/SMBCall" ${rootDir}/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
        send=$(grep "return_package/SMBReturn" ${rootDir}/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')

        echo ${dir} ${connect} ${receive} ${send}

        if [[ -n "$connect" ]]; then
            connect="${connect%\%}"
        else
            connect="< 1"
        fi

        if [[ -n "$receive" ]]; then
            receive="${receive%\%}"
        else
            receive="< 1"
        fi

        if [[ -n "$send" ]]; then
            send="${send%\%}"
        else
            send="< 1"
        fi

        # echo -e "\t\t${dir} & ${connect} & ${disconnect} & ${receive} & ${send} \\\\\\\\" >> ${outFile}
        echo -e "\t\t${dir} & ${connect} & ${receive} & ${send} \\\\\\\\" >> ${outFile}
    done
    echo -e "\t\t\\\\bottomrule" >> ${outFile}
    echo -e "\t\\\\end{tabular}" >> ${outFile}
    echo -e "\t\\\\label{tab:alloc_sampling}" >> ${outFile}
    echo -e "\\\\end{table}" >> ${outFile}
    echo ${END_LINE}
}

runBenchmark() {
    # generateLatexTableFromSVGsALLOC benchmark_results/1755695703 service_alloc_sampling benchmark_results/1755695703_alloc_sampling_table alloc
    # exit 1
    flamegraphPath=$1
    shift
    profilerPath=$1
    shift
    clientAbsolutePath=$1
    shift

    timestamp=$(date +%s)

    for clientsNr in "$@"; do
        cpuFlamegraphTitle="service_cpu_sampling"
        allocFlamegraphTitle="service_alloc_sampling"
        cacheMissFlamegraphTitle="service_cache_miss_sampling"
        cpuAllocFlamegraphTitle="service_cpu_alloc_sampling"
        nativeMemFlamegraphTitle="service_nativemem_sampling"
        lockFlamegraphTitle="service_lock_sampling"
        cpuAllocNativeMemLockFlamegraphTitle="service_cpu_alloc_nativemem_lock_sampling"

        servicePID=$(pgrep -f 'java -cp')

        ${profilerPath}/build/bin/asprof start -e cpu,alloc,nativemem,lock -o jfr -f ./${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr ${servicePID}

        if [ ! -d "benchmark_results/clients/${clientsNr}" ]; then
            mkdir -p benchmark_results/clients/${clientsNr}
        fi

        runClients $clientAbsolutePath $clientsNr

        wait

        ${profilerPath}/build/bin/asprof stop -o jfr -f ./${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr ${servicePID}

        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --cpu ${cpuFlamegraphTitle} ./benchmark_results/${timestamp}/${clientsNr}/cpu
        parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --alloc ${allocFlamegraphTitle} ./benchmark_results/${timestamp}/${clientsNr}/alloc
        # parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --nativemem ${nativeMemFlamegraphTitle} ./benchmark_results/${timestamp}/${clientsNr}/native_alloc
        # parseProfilingResult $profilerPath $flamegraphPath ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle} --lock ${lockFlamegraphTitle} ./benchmark_results/${timestamp}/${clientsNr}/lock

        rm ${timestamp}_${cpuAllocNativeMemLockFlamegraphTitle}.jfr

        convertSVGToPDF ${cpuFlamegraphTitle} ./benchmark_results/${timestamp}/${clientsNr}/cpu
        convertSVGToPDF ${allocFlamegraphTitle} ./benchmark_results/${timestamp}/${clientsNr}/alloc

        # convertSVGToPDF ${nativeMemFlamegraphTitle} ./benchmark_results/${timestamp}/${clientsNr}/native_alloc
        # convertSVGToPDF ${lockFlamegraphTitle} ./benchmark_results/${timestamp}/${clientsNr}/lock

        # generateTopTableFromCollapsed "./benchmark_results/${timestamp}/${clientsNr}/cpu/${cpuFlamegraphTitle}.collapsed" "./benchmark_results/${timestamp}/${clientsNr}/cpu" "cpu_${clientsNr}"

        # generateTopTableFromCollapsed "./benchmark_results/${timestamp}/${clientsNr}/alloc/${allocFlamegraphTitle}.collapsed" "./benchmark_results/${timestamp}/${clientsNr}/alloc" "alloc_${clientsNr}"

        echo "All $clientsNr clients have finished."
    done

    generateLatexTableFromSVGsCPU benchmark_results/${timestamp} service_cpu_sampling benchmark_results/${timestamp}_cpu_sampling_table cpu
    generateLatexTableFromSVGsALLOC benchmark_results/${timestamp} service_alloc_sampling benchmark_results/${timestamp}_alloc_sampling_table alloc

    echo "Benchmark completed."
}

runBenchmark $@

# docker exec -it --user user -e TERM=xterm-256color -w /home/user docker-dsp-library-1 bash

# git clone https://github.com/brendangregg/FlameGraph.git
# git clone https://github.com/async-profiler/async-profiler.git
# ./benchmark.sh /home/user/FlameGraph /home/user/async-profiler $(pwd) 500 750 1000 1500 2000 2500 3250 4000 5000 6000
