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
cgcreate -g memory:atc
log_path=/home/osdi/ae/results/fig4/atc
exe_file=/home/osdi/atc/trianglecount/build/lib/main
data_file=/home/osdi/datasets/com-friendster.ungraph.binary

stdbuf -o0 numactl --cpubind=0 --membind=0 cgexec -g memory:atc --sticky $exe_file -graph $data_file -v 65608367 -cgroup atc -r 100 | tee $log_path/atlas-100-raw
python3 /home/osdi/ae/results/fig4/atc/process.py 100
stdbuf -o0 numactl --cpubind=0 --membind=0 cgexec -g memory:atc --sticky $exe_file -graph $data_file -v 65608367 -cgroup atc -r 75 | tee $log_path/atlas-75-raw
python3 /home/osdi/ae/results/fig4/atc/process.py 75
stdbuf -o0 numactl --cpubind=0 --membind=0 cgexec -g memory:atc --sticky $exe_file -graph $data_file -v 65608367 -cgroup atc -r 50 | tee $log_path/atlas-50-raw
python3 /home/osdi/ae/results/fig4/atc/process.py 50
stdbuf -o0 numactl --cpubind=0 --membind=0 cgexec -g memory:atc --sticky $exe_file -graph $data_file -v 65608367 -cgroup atc -r 25 | tee $log_path/atlas-25-raw
python3 /home/osdi/ae/results/fig4/atc/process.py 25
stdbuf -o0 numactl --cpubind=0 --membind=0 cgexec -g memory:atc --sticky $exe_file -graph $data_file -v 65608367 -cgroup atc -r 13 | tee $log_path/atlas-13-raw
python3 /home/osdi/ae/results/fig4/atc/process.py 13

