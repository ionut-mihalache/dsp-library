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
    # for i in $(seq 1 $clientsNr); do
    #     php main.php &
    # done
}

runBenchmark() {
    profilerPath=$1
    shift
    clientAbsolutepath=$1
    shift

    for clientsNr in "$@"; do
        samplingTime=$((clientsNr <= 128 ? (clientsNr < 128 ? 10 : clientsNr) : (clientsNr < 1024 ? 30 : 60)))
        flamegraphTitle="service_benchmark_clients_${clientsNr}_sampling_${samplingTime}s"
        echo "Starting benchmark with $clientsNr clients, sampling time: $samplingTime seconds, flamegraph title: $flamegraphTitle"
        echo "Profiler path: $profilerPath/build/bin"
        # ${profilerPath}/build/bin/asprof -e cpu -d $samplingTime -f ${flamegraphTitle}.html &
        # profilerPath -e cpu -d
        runClients $clientAbsolutepath $clientsNr
        wait
        echo "All $clientsNr clients have finished."
    done
    echo "Benchmark completed."
}

runBenchmark $@
# ./build/bin/asprof -e cpu -d 50 -f cpu-flame-php.html 31421
# ./build/bin/asprof -e cpu -d 50 -f cpu-flame-java.html $(pgrep java)
