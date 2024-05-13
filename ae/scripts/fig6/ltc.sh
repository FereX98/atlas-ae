#!/bin/bash

function kill_mcd {
    sudo pkill -9 memcached > /dev/null 2>&1 
    sleep 5s
    memcached_pid=`pidof memcached`
    if [ ! -z $memcached_pid ]; then
        echo "memcached is still running, pid $memcached_pid"
        exit 1
    fi
}

kill_mcd

### set up memcached
echo "Setting parameters"
echo Y > /sys/kernel/debug/hermit/vaddr_swapout
echo Y > /sys/kernel/debug/hermit/batch_swapout
echo Y > /sys/kernel/debug/hermit/batch_io
echo Y > /sys/kernel/debug/hermit/batch_tlb
echo Y > /sys/kernel/debug/hermit/batch_account
echo Y > /sys/kernel/debug/hermit/bypass_swapcache
echo Y > /sys/kernel/debug/hermit/lazy_poll
echo Y > /sys/kernel/debug/hermit/speculative_io
echo N > /sys/kernel/debug/hermit/speculative_lock

echo N > /sys/kernel/debug/hermit/async_prefetch
echo N > /sys/kernel/debug/hermit/prefetch_direct_poll
echo N > /sys/kernel/debug/hermit/prefetch_direct_map
echo N > /sys/kernel/debug/hermit/prefetch_populate
echo N > /sys/kernel/debug/hermit/prefetch_always_ascend
echo N > /sys/kernel/debug/hermit/atl_card_prof
echo N > /sys/kernel/debug/hermit/atl_card_prof_print
echo N > /sys/kernel/debug/hermit/bks/direct_swap

echo 0 > /sys/kernel/mm/swap/readahead_win
echo 1 > /sys/kernel/debug/hermit/sthd_cnt
echo 1 > /sys/kernel/debug/hermit/populate_work_cnt
echo 1 > /sys/kernel/debug/hermit/min_sthd_cnt
echo 26 > /sys/kernel/debug/hermit/atl_card_prof_thres
echo 0 > /sys/kernel/debug/hermit/atl_card_prof_low_thres


### prepare to run memcached
exec_dir=/home/osdi/mcd/atlas-apps/memcached
client_host=memserver52
client_dir=/home/osdi/caladan

# create cgroup
mkdir /sys/fs/cgroup/memory/memcached

function run_server {
    local_size=$1
    local_ratio=$2

    sudo pkill -9 memcached > /dev/null 2>&1 
    sleep 5s
    memcached_pid=`pidof memcached`
    if [ ! -z $memcached_pid ]; then
        echo "memcached is still running, pid $memcached_pid"
        exit 1
    fi

    echo "Running memcached, local memory size $local_size, local memory ratio $local_ratio"
    echo ${local_size} | sudo tee /sys/fs/cgroup/memory/memcached/memory.limit_in_bytes
    actual_exec_dir=${exec_dir}
    echo "mcd to run is ${actual_exec_dir}/memcached}"
    sleep 3
    cgexec -g memory:memcached --sticky taskset -c 35-46 ${actual_exec_dir}/memcached -m 102400 -p 11211 -t 12 -o hashpower=29,no_hashexpand,no_lru_crawler,no_lru_maintainer -c 32768 -b 32768 &
    sleep 5
}

function ssh_run() {
    echo "ssh $client_host \"$@\""
    ssh ${client_host} "$@"
    if [ $? -ne 0 ]; then
        echo "-- ssh_run \"$@\" failed"
        exit 1
    fi
}

function run_client {
    workload=$1
    local_ratio=$2
    target_mops=$3

    ssh_run "cd ${client_dir}; bash run-latency.sh at ${target_mops}"

    sleep 1
    scp ${client_host}:/home/osdi/caladan/logs/at-mcdcll-${target_mops}.log /home/osdi/ae/results/fig6a/atlas-${target_mops}
    if [ $? -ne 0 ]; then
        echo "scp workloads failed"
        exit 1
    fi
    sleep 1
}

function run_workload {
    mem_p100=100G
    workload=$1
    mem_p75=$2
    mem_p50=$3
    mem_p25=$4
    mem_p13=$5

    target_mops_arr=(0.1 0.2 0.3 0.4 0.5 1.0 1.5 2.0 2.5 3.0 3.5 4.0 4.4 4.6)

    echo "Running workload $workload, with 50% memory $mem_p50, 25% memory $mem_p25, 13% memory $mem_p13"

    # setup iokerneld on client
    ssh_run "cd ${client_dir}; bash setup.sh at"
    sleep 3

    for target_mops in ${target_mops_arr[@]}; do
        run_server $mem_p25  25
        run_client $workload 25 $target_mops
        kill_mcd
    done

    python3 /home/osdi/ae/results/fig6a/process.py
}

# MCD-CL: 6963+4096M
run_workload cl 9318M 7577M 5836M 4966M
