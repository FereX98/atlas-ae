work_dir=$(dirname $(readlink -f "$0"))
scripts_dir=${work_dir}/../scripts
log_dir=${work_dir}/logs
if [ ! -d $log_dir ]; then
    echo "Creating log directory $log_dir"
    mkdir -p $log_dir
fi

exec_dir=${work_dir}

cd ${work_dir}
make clean && make -j

cgcreate -g memory:snappy
function run_compress {
    local_size=$1
    local_ratio=$2
    kernel_tag=$3
    echo "Running compress, local memory size $local_size, local memory ratio $local_ratio"
    if [ $kernel_tag = "fastswap" ]; then
        echo "Using the fastswap configuration!"
        # bash ${scripts_dir}/setup_fastswap.sh
        bash ${scripts_dir}/setup_hermit.sh
        echo 1 > /sys/kernel/debug/hermit/sthd_cnt
    elif [ $kernel_tag = "hermit" ]; then
        echo "Using the hermit configuration!"
        bash ${scripts_dir}/setup_hermit.sh
    elif [ $kernel_tag = "atlas" ]; then
        echo "Using the atlas configuration!"
        bash ${scripts_dir}/setup_atlas.sh
    else
        echo "Unknown kernel tag $kernel_tag"
        exit 1
    fi
    cgset -r memory.limit_in_bytes=${local_size} snappy
    numactl --cpubind=1 --membind=1 cgexec -g memory:snappy ${exec_dir}/main > $log_dir/compress.${kernel_tag}.log.${local_ratio} 2>&1
}

# working set 18432 MB

run_compress 100G p100 hermit
run_compress 9216M p50 hermit
run_compress 4608M p25 hermit
run_compress 2304M p12.5 hermit

run_compress 100G p100 atlas
run_compress 9216M p50 atlas
run_compress 4608M p25 atlas
run_compress 2304M p12.5 atlas

run_compress 100G p100 fastswap
run_compress 9216M p50 fastswap
run_compress 4608M p25 fastswap
run_compress 2304M p12.5 fastswap
