#!/bin/bash

### setup
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
echo 6 > /sys/kernel/debug/hermit/sthd_cnt
echo 1 > /sys/kernel/debug/hermit/populate_work_cnt
echo 1 > /sys/kernel/debug/hermit/min_sthd_cnt
echo 8 > /sys/kernel/debug/hermit/atl_card_prof_thres
echo 0 > /sys/kernel/debug/hermit/atl_card_prof_low_thres

### Running
target_mops_arr=(0.2)
### !!!update code before changing this!!! ###
num_threads_arr=(20)

sudo cgcreate -t $USER -a $USER -g memory:/hash
log_file="/home/osdi/ae/results/fig4/ws"
app_path="/home/osdi/ws/atlas-apps/hashtable/atlas"

function run {
    mem=$1
    ratio=$2
    nt=$3
    for target_mops in ${target_mops_arr[@]}; do
        #for iteration in {2..4}
        for iteration in 1
        do
            for num_threads in ${num_threads_arr[@]}
            do
                executable="zipf_web-ol-cdf"
                nontemp="false"
                sudo pkill -9 zipf_web
                if [[ "$nt" == "nt" ]]; then
                    nontemp="true"
                fi
                sed "s/const bool non_temporal = .*/const bool non_temporal = ${nontemp};/g" ${app_path}/zipf_web-offered-load-cdf.cpp -i
                sed "s/memory.limit_in_bytes=.*hash/memory.limit_in_bytes=${mem} hash/g" ${app_path}/zipf_web-offered-load-cdf.cpp -i
                sed "s/constexpr double target_mops = .*/constexpr double target_mops = $target_mops;/g" ${app_path}/zipf_web-offered-load-cdf.cpp -i

                cd $app_path
                make clean
                make zipf_web-ol-cdf
                cgset -r memory.limit_in_bytes=100G hash
                if [ $? -ne 0 ]; then
                    echo "Error: cgset failed"
                    exit 1
                fi
                cgexec -g memory:hash --sticky taskset -c 24-41 ${app_path}/${executable}
                if [ $? -ne 0 ]; then
                    echo "Error: run failed"
                    exit 1
                fi
            done
        done
    done
}

# overhead: 1650m: DS in test class to be pinned: 40 + 320 + 640 + 1300*0.5
# 25%: (5+6)*1024+1650m
run 12914m 25 nt
