work_dir=$(dirname $(readlink -f "$0"))
log_dir=${work_dir}/logs
if [ ! -d $log_dir ]; then
    echo "Creating log directory $log_dir"
    mkdir -p $log_dir
fi

exec_dir=${work_dir}/phoenix/phoenix-2.0/tests/kmeans

cgcreate -g memory:kmeans

function run {
    local_size=$1
    local_ratio=$2
    for i in {1..1} 
    do
        echo "Running kmeans, local memory size $local_size, local memory ratio $local_ratio, iter $i"
        cgset -r memory.limit_in_bytes=${local_size} kmeans
        cgexec -g memory:kmeans --sticky ${exec_dir}/kmeans -d 3 -c 1000 -p 500000000 -s 1000000 > $log_dir/kmeans.log.iter${i}.${local_ratio} 2>&1
    done 
}

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/Paris/third_party/jemalloc/lib

# working set 12466956 KB 
run 100G p100
run 6087M p50
run 3044M p25
run 1522M p12.5
