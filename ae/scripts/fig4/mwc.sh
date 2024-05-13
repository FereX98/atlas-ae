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
(echo 0 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo) > /dev/null
echo 1 > /sys/kernel/debug/hermit/populate_work_cnt
echo 1 > /sys/kernel/debug/hermit/min_sthd_cnt
echo 16 > /sys/kernel/debug/hermit/atl_card_prof_thres

### Running
cgcreate -g memory:mwc
log_path=/home/osdi/ae/results/fig4/mwc
exe_file=/home/osdi/mwc/wordcount/obj/wc
data_file=/home/osdi/datasets/news.2009.en.shuffled

cgset -r memory.limit_in_bytes=100G mwc
stdbuf -o0 cgexec -g memory:mwc numactl --cpubind=0 --membind=0 $exe_file $data_file -p 8 -r 64000000 -m 65536  | tee ${log_path}/atlas-100-raw
python3 /home/osdi/ae/results/fig4/mwc/process.py 100
cgset -r memory.limit_in_bytes=11485M mwc
stdbuf -o0 cgexec -g memory:mwc numactl --cpubind=0 --membind=0 $exe_file $data_file -p 8 -r 64000000 -m 65536  | tee ${log_path}/atlas-75-raw
python3 /home/osdi/ae/results/fig4/mwc/process.py 75
cgset -r memory.limit_in_bytes=11163M mwc
stdbuf -o0 cgexec -g memory:mwc numactl --cpubind=0 --membind=0 $exe_file $data_file -p 8 -r 64000000 -m 65536  | tee ${log_path}/atlas-50-raw
python3 /home/osdi/ae/results/fig4/mwc/process.py 50
cgset -r memory.limit_in_bytes=10841M mwc
stdbuf -o0 cgexec -g memory:mwc numactl --cpubind=0 --membind=0 $exe_file $data_file -p 8 -r 64000000 -m 65536  | tee ${log_path}/atlas-25-raw
python3 /home/osdi/ae/results/fig4/mwc/process.py 25
cgset -r memory.limit_in_bytes=10680M mwc
stdbuf -o0 cgexec -g memory:mwc numactl --cpubind=0 --membind=0 $exe_file $data_file -p 8 -r 64000000 -m 65536  | tee ${log_path}/atlas-13-raw
python3 /home/osdi/ae/results/fig4/mwc/process.py 13
(echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo) > /dev/null
