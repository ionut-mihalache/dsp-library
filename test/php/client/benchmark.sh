# usage: ./benchmark.sh profiler_absolute_path client_absolute_path clientsNr1 clientsNr2 ...
# Example: ./benchmark.sh /path/to/profiler /path/to/client 1 2 3 4 5
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 profiler_absolute_path client_absolute_path clientsNr1 clientsNr2 ..."
    exit 1
fi

runClients() {
    clientsNr=$1
    for i in $(seq 1 $clientsNr); do
        php main.php &
    done
}

runBenchmark() {
    profilerPath=$1
    shift
    client_absolute_path=$1
    shift

    echo "Running $clientsNr clients..."
    for clientsNr in "$@"; do
        echo "Starting $clientsNr clients..."
        # runClients $clientsNr
        wait
        echo "All $clientsNr clients have finished."
    done
    echo "Benchmark completed."
}

runBenchmark $@
# ./build/bin/asprof -e cpu -d 50 -f cpu-flame-php.html 31421
