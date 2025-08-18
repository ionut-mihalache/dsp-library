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
        mkdir -p ${outPath}
    fi

    flamegraphAttributes="--width 3000 --minWidth 10 --fontsize 14"
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

generateLatexTableFromSVGs() {
    svgFile=$1
    outFile=$2
    sampleDir=$3

    WIDTH=80

    PADDING=$(( (WIDTH - 10 - ${#sampleDir}) / 2 ))
    FILL=$(printf '%*s' "$PADDING" '' | tr ' ' '=')

    START_LINE="${FILL} START OF ${sampleDir^^} RESULTS ${FILL}"
    END_LINE="${FILL} END OF ${sampleDir^^} RESULTS ${FILL}"

    # If the length is odd, add one more '=' to reach 80 chars
    while [ ${#START_LINE} -lt $WIDTH ]; do START_LINE+="="; done
    while [ ${#END_LINE} -lt $WIDTH ]; do END_LINE+="="; done

    echo ${START_LINE}
    for dir in $(ls benchmark_results/1755358810 | sort -n); do
        connect=$(grep "ConnectThread" benchmark_results/1755358810/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
        disconnect=$(grep "DisconnectThread" benchmark_results/1755358810/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
        receive=$(grep "receiveCall" benchmark_results/1755358810/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
        send=$(grep "sendReturn" benchmark_results/1755358810/${dir}/${sampleDir}/${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')

        echo ${dir} ${connect} ${disconnect} ${receive} ${send}
    done
    echo ${END_LINE}

    # $connect=$(grep "ConnectThread" ${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
    # $disconnect=$(grep "DisconnectThread" ${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
    # $receive=$(grep "receiveCall" ${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
    # $send=$(grep "sendReturn" ${svgFile}.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}')
}

generateTopTableFromCollapsed() {
    collapsedFile=$1
    outPath=$2
    tableLabel=$3

    totalSamples=$(awk '{sum+=$NF} END {print sum}' "$collapsedFile")
    if [ -z "$totalSamples" ] || [ "$totalSamples" -eq 0 ]; then
        echo "Warning: $collapsedFile has 0 samples. Skipping table."
        return
    fi


    grep "ConnectThread" 
    grep "DisconnectThread" service_cpu_sampling.svg | awk -F'[()]' '{split($2, a, "samples, "); print a[2]}'

    awk -v total="$totalSamples" -v label="$tableLabel" '
    {
        count = $NF
        sub(/[ \t]+[0-9]+$/, "", $0)
        n = split($0, parts, ";")
        method = parts[n]
        counts[method] += count
    }
    END {
        printf "\\begin{table}[htbp]\n"
        printf "\\caption{All methods by sample count (%d samples)}\n", total
        printf "\\centering\n"
        printf "\\begin{tabular}{@{}p{7.4cm}rr@{}}\n"
        printf "\\toprule\nMethod & Samples & \\%% \\\\\n\\midrule\n"
        PROCINFO["sorted_in"] = "@val_num_desc"
        for (m in counts) {
        cnt = counts[m]
        pct = (cnt/total)*100

        g = m
        gsub(/\\/, "\\\\textbackslash{}", g)
        gsub(/_/,"\\\\_", g)
        gsub(/#/,"\\\\#", g)
        gsub(/%/,"\\\\%", g)
        gsub(/&/,"\\\\&", g)
        gsub(/\{/,"\\\\{", g)
        gsub(/\}/,"\\\\}", g)
        gsub(/\$/,"\\\\$", g)

        # Add break opportunities (properly) at "/" and "."
        gsub(/[\/.]/, "&keep[$0]&", g)

        # Wrap in \texttt{}
        formatted = "\\texttt{" g "}"

        printf "%s & %d & %.2f\\\\%% \\\\\n", formatted, cnt, pct
        }
        printf "\\bottomrule\n\\end{tabular}\n"
        printf "\\label{tab:%s}\n\\end{table}\n", label
    }
    ' "$collapsedFile" > "${outPath}/table_${tableLabel}.tex"
}

runBenchmark() {
    generateLatexTableFromSVGs service_cpu_sampling 2 cpu
    generateLatexTableFromSVGs service_alloc_sampling 2 alloc
    exit 1
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
    echo "Benchmark completed."
}

runBenchmark $@

# docker exec -it --user user -e TERM=xterm-256color -w /home/user docker-dsp-library-1 bash

# git clone https://github.com/brendangregg/FlameGraph.git
# git clone https://github.com/async-profiler/async-profiler.git
# ./benchmark.sh /home/user/FlameGraph /home/user/async-profiler $(pwd) 500 750 1000 1500 2000 2500 3250 4000
