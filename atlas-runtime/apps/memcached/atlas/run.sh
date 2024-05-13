work_dir=$(dirname $(readlink -f "$0"))
log_dir=${work_dir}/logs
if [ ! -d $log_dir ]; then
    echo "Creating log directory $log_dir"
    mkdir -p $log_dir
fi

bash $work_dir/../scripts/setup_hermit.sh

exec_dir=${work_dir}/memcached

cgcreate -g memory:memcached

function run_server {
    local_size=$1
    local_ratio=$2

    pkill memcached > /dev/null 2>&1 
    sleep 5s
    memcached_pid=`pidof memcached`
    if [ ! -z $memcached_pid ]; then
        echo "memcached is still running, pid $memcached_pid"
        exit 1
    fi

    echo "Running memcached, local memory size $local_size, local memory ratio $local_ratio"
    cgset -r memory.limit_in_bytes=${local_size} memcached
    numactl --cpubind=1 --membind=1 cgexec -g memory:memcached --sticky \
        ${exec_dir}/memcached -m 102400 -p 11211 -t 4 -o hashpower=27,no_hashexpand,no_lru_crawler,no_lru_maintainer -d
}

function run_client {
    workload=$1
    local_ratio=$2
    bash $work_dir/../scripts/ycsb_client.sh $workload > ${log_dir}/ycsb_${workload}_${local_ratio}.log 2>&1
    if [ $? -ne 0 ]; then
        echo "ycsb client failed"
        exit 1
    fi
}

function run_workload {
    mem_p100=100G
    workload=$1
    mem_p50=$2
    mem_p25=$3
    mem_p13=$4

    echo "Running workload $workload, with 50% memory $mem_p50, 25% memory $mem_p25, 13% memory $mem_p13"

    run_server $mem_p100 p100
    run_client $workload p100

    run_server $mem_p50 p50
    run_client $workload p50

    run_server $mem_p25 p25
    run_client $workload p25

    run_server $mem_p13 p13
    run_client $workload p13
}

# `readonly` working set 6932200 KB 
run_workload atlas_readonly 3385M 1692M 846M

# `read_dominant` working set 8108404 KB 
run_workload atlas_read_dominant 3959M 1980M 990M

# `insert_dominant` working set 10461736 KB 
run_workload atlas_insert_dominant 5108M 2554M 1277M