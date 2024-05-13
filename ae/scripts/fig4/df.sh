#!/bin/bash

### Set up variables
echo Y > /sys/kernel/debug/hermit/vaddr_swapout
echo Y > /sys/kernel/debug/hermit/batch_swapout
echo Y > /sys/kernel/debug/hermit/batch_io
echo Y > /sys/kernel/debug/hermit/batch_tlb
echo Y > /sys/kernel/debug/hermit/batch_account
echo Y > /sys/kernel/debug/hermit/bypass_swapcache
echo Y > /sys/kernel/debug/hermit/lazy_poll
echo Y > /sys/kernel/debug/hermit/speculative_io
echo N > /sys/kernel/debug/hermit/speculative_lock

echo Y > /sys/kernel/debug/hermit/async_prefetch
echo Y > /sys/kernel/debug/hermit/prefetch_direct_poll
echo Y > /sys/kernel/debug/hermit/prefetch_direct_map
echo Y > /sys/kernel/debug/hermit/prefetch_populate
echo Y > /sys/kernel/debug/hermit/prefetch_always_ascend
echo N > /sys/kernel/debug/hermit/bks/direct_swap
echo N > /sys/kernel/debug/hermit/atl_card_prof
echo N > /sys/kernel/debug/hermit/atl_card_prof_print

echo 256 > /sys/kernel/mm/swap/readahead_win
echo 16 > /sys/kernel/debug/hermit/sthd_cnt
echo 8 > /sys/kernel/debug/hermit/populate_work_cnt
echo 5 > /sys/kernel/debug/hermit/min_sthd_cnt


### Running
dataframe_path=/home/osdi/df/atlas-apps/dataframe
cache_sizes=(4 8 16 23 40)
ratios=(13 25 50 75 100)
sudo pkill -9 main

log_dir=/home/osdi/ae/results/fig4/df
if [ ! -d $log_dir ]; then
    echo "Creating log directory $log_dir"
    mkdir -p $log_dir
fi

cgcreate -g memory:dataframe
echo 1 > /sys/fs/cgroup/memory/dataframe/memory.early_populate
exec_path=${dataframe_path}/build/bin/main
if [ ! -f $exec_path ]; then
    echo "the dataframe app: ${exec_path} does not exist, please build dataframe first"
    exit 1
fi

for i in {0..4}
do
    cache_size=${cache_sizes[i]}
    ratio=${ratios[i]}
    cgset -r memory.limit_in_bytes=${cache_size}g dataframe
    echo "Run dataframe with cache size ${cache_size}g, ${ratio} percent local memory"
    taskset -c 24-27 cgexec -g memory:dataframe --sticky ${exec_path} | tee -a ${log_dir}/atlas-${ratio}-raw
    python3 /home/osdi/ae/results/fig4/df/process.py ${ratio}

done