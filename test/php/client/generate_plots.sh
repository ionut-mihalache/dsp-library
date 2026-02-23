lastAQUAbenchmark=$(ls benchmark_results/clients/ | grep -o "[0-9]\+" | sort -rn | head -n 1)
lastUDSbenchmark=$(ls uds/benchmark_results/clients/ | grep -o "[0-9]\+" | sort -rn | head -n 1)
lastZMQbenchmark=$(ls zeromq/benchmark_results/clients/ | grep -o "[0-9]\+" | sort -rn | head -n 1)

# echo $lastAQUAbenchmark
# echo $lastUDSbenchmark
# echo $lastZMQbenchmark

metrics2Path=$1
metrics3Path=$2

if [ ! -d ${metrics2Path} ]; then
    mkdir -p ${metrics2Path}
fi
if [ ! -d ${metrics3Path} ]; then
    mkdir -p ${metrics3Path}
fi

source venv/bin/activate
python3 create_combined_plots.py ${metrics2Path} benchmark_results/clients/$lastAQUAbenchmark/client_benchmark.csv uds/benchmark_results/clients/$lastUDSbenchmark/client_benchmark.csv
python3 create_combined_plots.py ${metrics3Path} benchmark_results/clients/$lastAQUAbenchmark/client_benchmark.csv uds/benchmark_results/clients/$lastUDSbenchmark/client_benchmark.csv zeromq/benchmark_results/clients/$lastZMQbenchmark/client_benchmark.csv
deactivate
