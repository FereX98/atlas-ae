## Memcached

### Prepare
1. Install the ycsb client in another server (different from the cpu server)
2. Set the `client_host`, `client_ycsb_dir`, `server_host`, `server_port` in `scripts/ycsb_client.sh`
```
client_host=10.208.130.16
client_ycsb_dir=/home/chenlei/ycsb-0.17.0
server_host=10.208.130.22
server_port=11211
```
2. Modify the `ATLAS_PATH` in `atlas_patch/0004-chore-update-Makefile.patch` and `origin_patch/0002-feat-use-jemalloc.patch`
```
ATLAS_PATH = /ssd/chenlei/atlas/Paris
```

### Run Origin (without paging optimizations)
1. build memcached
```
cd origin
bash setup.sh
```
2. run (Note. it takes ~1 hour to run 3 workloads, and see the details in `workloads`)
```
bash run.sh
```

### Run Hermit (with paging optimizations)
1. build memcached
```
cd hermit
bash setup.sh
```
2. run (Note. it takes ~1 hour to run 3 workloads, and see the details in `workloads`)
```
bash run.sh
```

### Run Atlas
1. build memcached
```
cd hermit
bash setup.sh
```
2. run (Note. it takes ~1 hour to run 3 workloads, and see the details in `workloads`)
```
bash run.sh
```