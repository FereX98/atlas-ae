work_dir=$(dirname $(readlink -f "$0"))
# data can be copied from linguine: /mnt/ssd/shiliu/dataset
data=${HOME}/dataset/twitterU.sg
if [ ! -f $data ]; then
    echo "Input data ${data} not exists. Please download the data first, and set the path"
    exit 1
fi

log_dir=${work_dir}/logs
if [ ! -d $log_dir ]; then
    echo "Creating log directory $log_dir"
    mkdir -p $log_dir
fi

exec_dir=${work_dir}/gapbs

cgcreate -g memory:gapbs

function run {
    local_size=$1
    local_ratio=$2
    echo "Running triangle_count, local memory size $local_size, local memory ratio $local_ratio"
    cgset -r memory.limit_in_bytes=${local_size} gapbs
    cgexec -g memory:gapbs --sticky ${exec_dir}/tc -f $data -n1 -a > $log_dir/triangle_count.log.${local_ratio} 2>&1
    if [ $? -ne 0 ]; then
        echo "Failed to run triangle_count, Please check the log"
        exit 1
    fi
}

# working set 9784 MB 
run 100G p100
run 4892M p50
run 2446M p25
run 1223M p12.5
