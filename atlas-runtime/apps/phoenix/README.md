## Phoenix

### Prepare
1. Download the dataset for `WordCount`
```
wget https://statmt.org/wmt11/training-monolingual-news-2009.tgz
```
2. Set the dataset path in `atlas/run_wordcout.sh` and `origin/run_wordcount.sh`
3. Build jemalloc
```
cd <atlas-root>/third_party/jemalloc/
./autogen.sh
make -j
```

Edit `JEMALLOC_DIR` in `0001-fix-fix-performance-bug-in-the-far-memory-context.patch`.

### Run Origin
(PS. the Origin is running on `hermit` kernel actually (optimizations enabled). Install the `fastswap-port` kernel when compared with fastswap)
1. build phoenix
```
cd origin
bash setup.sh
```
2. run `WordCount` (Warning: long running time, ~500s in 12.5% local memory)
```
bash run_wordcount.sh
```
3. run `KMeans`
```
bash run_kmeans.sh
```

### Run Atlas
1. build phoenix
```
cd atlas
bash setup.sh
```
2. run `WordCount` (Warning: long running time, ~70s in 12.5% local memory)
```
bash run_wordcount.sh
```
3. run `KMeans`
```
bash run_kmeans.sh
```