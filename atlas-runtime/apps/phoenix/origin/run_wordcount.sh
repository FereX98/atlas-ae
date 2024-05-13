# data can be copied from linguine: /mnt/ssd/shiliu/dataset
data=${HOME}/dataset/news.2009.en.shuffled
if [ ! -f $data ]; then
    echo "Input data ${data} not exists. Please download the data first, and set the path"
    exit 1
fi

work_dir=$(dirname $(readlink -f "$0"))
log_dir=${work_dir}/logs
if [ ! -d $log_dir ]; then
    echo "Creating log directory $log_dir"
    mkdir -p $log_dir
fi

exec_dir=${work_dir}/phoenix/phoenix-2.0/tests/word_count

cgcreate -g memory:wordcount

function run {
    local_size=$1
    local_ratio=$2
    echo "Running word_count, local memory size $local_size, local memory ratio $local_ratio"
    cgset -r memory.limit_in_bytes=${local_size} wordcount
    cgexec -g memory:wordcount ${exec_dir}/word_count $data > $log_dir/work_count.log.${local_ratio} 2>&1
}

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/Paris/third_party/jemalloc/lib

# working set 6567448 KB 
run 100G p100
run 3207M p50
run 1603M p25
run 802M p12.5
