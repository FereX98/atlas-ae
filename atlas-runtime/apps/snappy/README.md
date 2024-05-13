## Snappy

### Prepare
1. Download the dataset
```
cd setup
bash run.sh
```
2. Set the dataset path in `compress/main.cpp` and `uncompress/main.cpp`

### Run Compress
```
cd compress
bash run.sh
```

### Run Uncompress
```
cd uncompress
bash run.sh
```

**Note:** The fastswap's results are generated using `hermit` kernel. Please install the `fastswap-port` kernel and rerun the `compress` & `uncompress` to get the correct results.