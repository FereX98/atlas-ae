#!/bin/bash

### Set up
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
echo 16 > /sys/kernel/debug/hermit/sthd_cnt
echo 1 > /sys/kernel/debug/hermit/populate_work_cnt
echo 1 > /sys/kernel/debug/hermit/min_sthd_cnt
echo 16 > /sys/kernel/debug/hermit/atl_card_prof_thres

### Running
cgcreate -g memory:dg
log_path=/home/osdi/ae/results/fig4/gpr
exe_file=/home/osdi/gpr/AIFM-Atlas/aifm/pagerank/atlas/build/lib/main
data_file=/home/osdi/datasets/twitter_rv.binary

stdbuf -o0 cgexec -g memory:dg --sticky taskset -c 0-15 $exe_file -graph $data_file -v 61578415 -cgroup dg -r 100 | tee $log_path/atlas-100-raw
python3 /home/osdi/ae/results/fig4/gpr/process.py 100
stdbuf -o0 cgexec -g memory:dg --sticky taskset -c 0-15 $exe_file -graph $data_file -v 61578415 -cgroup dg -r 75  | tee $log_path/atlas-75-raw
python3 /home/osdi/ae/results/fig4/gpr/process.py 75
stdbuf -o0 cgexec -g memory:dg --sticky taskset -c 0-15 $exe_file -graph $data_file -v 61578415 -cgroup dg -r 50  | tee $log_path/atlas-50-raw
python3 /home/osdi/ae/results/fig4/gpr/process.py 50
stdbuf -o0 cgexec -g memory:dg --sticky taskset -c 0-15 $exe_file -graph $data_file -v 61578415 -cgroup dg -r 25  | tee $log_path/atlas-25-raw
python3 /home/osdi/ae/results/fig4/gpr/process.py 25
stdbuf -o0 cgexec -g memory:dg --sticky taskset -c 0-15 $exe_file -graph $data_file -v 61578415 -cgroup dg -r 13  | tee $log_path/atlas-13-raw
python3 /home/osdi/ae/results/fig4/gpr/process.py 13
