lastAQUAbenchmark=$(ls benchmark_results/clients/ | grep -o "[0-9]\+" | sort -rn | head -n 1)
lastUDSbenchmark=$(ls uds/benchmark_results/clients/ | grep -o "[0-9]\+" | sort -rn | head -n 1)
lastZMQbenchmark=$(ls zeromq/benchmark_results/clients/ | grep -o "[0-9]\+" | sort -rn | head -n 1)

echo $lastAQUAbenchmark
echo $lastUDSbenchmark
echo $lastZMQbenchmark

if [ ! -d "metrics2" ]; then
    mkdir -p metrics2
fi
if [ ! -d "metrics3" ]; then
    mkdir -p metrics3
fi

source venv/bin/activate
python3 create_combined_plots.py ./metrics2 benchmark_results/clients/1770282471/client_benchmark.csv uds/benchmark_results/clients/1770282267/client_benchmark.csv
python3 create_combined_plots.py ./metrics3 benchmark_results/clients/1770282471/client_benchmark.csv uds/benchmark_results/clients/1770282267/client_benchmark.csv
deactivate

# ./benchmark.sh /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) $(date +%s) 128 256 384 512 640 758 886 1024