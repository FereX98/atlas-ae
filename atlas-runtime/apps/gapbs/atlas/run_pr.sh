work_dir=$(dirname $(readlink -f "$0"))
# data can be copied from linguine: /mnt/ssd/shiliu/dataset
data=${HOME}/dataset/twitter.sg
if [ ! -f $data ]; then
    echo "Input data ${data} not exists. Please download the data first, and set the path"
    exit 1
fi

log_dir=${work_dir}/logs
if [ ! -d $log_dir ]; then
    echo "Creating log directory $log_dir"
    mkdir -p $log_dir
fi

bash $work_dir/../scripts/setup_atlas.sh
echo 64 > /sys/kernel/mm/swap/readahead_win

exec_dir=${work_dir}/gapbs

cgcreate -g memory:gapbs

function run {
    local_size=$1
    local_ratio=$2
    echo "Running page_rank, local memory size $local_size, local memory ratio $local_ratio"
    cgset -r memory.limit_in_bytes=${local_size} gapbs
    cgexec -g memory:gapbs --sticky ${exec_dir}/pr -f $data -i1000 -t1e-4 -n3 > $log_dir/page_rank.log.${local_ratio} 2>&1
    if [ $? -ne 0 ]; then
        echo "Failed to run page_rank, Please check the log"
        exit 1
    fi
}

# working set 12600 MB 

run 100G p100
echo 256 > /sys/kernel/mm/swap/readahead_win
run 6300M p50
echo 64 > /sys/kernel/mm/swap/readahead_win
run 3150M p25
run 1575M p12.5
