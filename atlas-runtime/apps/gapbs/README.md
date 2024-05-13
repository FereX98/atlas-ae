## GAPBS

### Prepare
1. Download the twitter dataset (Note. modify `gapbs/benchmark/bench.mk` as we only need the twitter dataset)
```
git submodule update --init --recursive
cd gapbs
make bench-graphs
```
2. Move the dataset into `dataset` folder

### Run Origin
(Note. the Origin is running on `hermit` kernel actually (optimizations enabled). Install the `fastswap-port` kernel when compared with fastswap)
1. build gapbs
```
cd origin
bash setup.sh
```
2. run `TriangleCount`
```
bash run_tc.sh
```
3. run `PageRank`
```
bash run_pr.sh
```

### Run Atlas
1. build gapbs
```
cd atlas
bash setup.sh
```
2. run `TriangleCount`
```
bash run_tc.sh
```
3. run `PageRank`
```
bash run_pr.sh
```