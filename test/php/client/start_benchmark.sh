rm /dev/shm/*

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ubuntu/dsp-library

cd /home/ubuntu/dsp-library/test/java/service/xslt-transformation
make clean
make
make run &
aquaPid=$!
sleep 2
cd -

for i in $(seq 1 9);
do
    ./benchmark.sh false /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024
done

./benchmark.sh true /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024

kill $aquaPid

cd /home/ubuntu/dsp-library/test/java/service/xslt-transformation/src/uds
make clean
make
make run &
udsPid=$!
sleep 2
cd -

cd ./uds

for i in $(seq 1 9);
do
    ./benchmark.sh false /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024
done

./benchmark.sh true /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024

cd -

kill $udsPid

cd /home/ubuntu/dsp-library/test/java/service/xslt-transformation/src/zeromq
make clean
make
make run &
zeromqPid=$!
sleep 2
cd -

cd ./zeromq

for i in $(seq 1 9);
do
    ./benchmark.sh false /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024
done

./benchmark.sh true /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) 128 256 384 512 640 758 886 1024

cd -

kill $zeromqPid


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

# ./benchmark.sh /home/$(whoami)/FlameGraph /home/$(whoami)/async-profiler $(pwd) $(date +%s) 128 256 384 512 640 758 886 1024