#!/bin/bash
client_host=10.208.130.16
client_ycsb_dir=/home/chenlei/ycsb-0.17.0
server_host=10.208.130.22
server_port=11211

function ssh_run() {
    echo "ssh $client_host \"$@\""
    ssh ${client_host} "$@"
    if [ $? -ne 0 ]; then
        echo "-- ssh_run \"$@\" failed"
        exit 1
    fi
}

function ycsb_load() {
    workload=$1
    echo "ycsb load $workload"
    ssh_run "cd ${client_ycsb_dir}; ./bin/ycsb.sh load memcached -s -P workloads/${workload} -p \"memcached.hosts=${server_host}:${server_port} -threads 48\""
}

function ycsb_run() {
    workload=$1
    echo "ycsb run $workload"
    ssh_run "cd ${client_ycsb_dir}; ./bin/ycsb.sh run memcached -s -P workloads/${workload} -p \"memcached.hosts=${server_host}:${server_port} -threads 48\""
}

work_dir=$(dirname $(readlink -f "$0"))
scp ${work_dir}/../workloads/* ${client_host}:${client_ycsb_dir}/workloads
if [ $? -ne 0 ]; then
    echo "scp workloads failed"
    exit 1
fi

workload=$1
ycsb_load ${workload}
ycsb_run ${workload}
