## Ligra Experiment

**Ligra** is a lightweight graph processing framework based on shared memory.

In this experiment, we test the PageRank performance.

### Prepare Input
We use the [wikipedia_link_en](http://konect.cc/networks/wikipedia_link_en/) dataset as input. The ```import_wikipedia.py``` is necessary to generate graph file with the adjacency-graph format used by Ligra.
```
wget http://konect.cc/files/download.tsv.wikipedia_link_en.tar.bz2
tar -xvf download.tsv.wikipedia_link_en.tar.bz2
python3 import_wikipedia.py wikipedia_link_en/out.wikipedia_link_en ligra/inputs/ligra_wikipedia_link_en
```

### Build Ligra with Paris
```
cd ./ligra
git apply ../ligra_paris.patch
export OPENMP=1
export PARIS_PATH=<paris_root_path>
make -j
cd ..
```

### Run *PageRank*
```
cd ./ligra/apps

cgcreate -g memory:pagerank
cgset -r memory.limit_in_bytes=1g pagerank
cgexec -g memory:pagerank --sticky ./PageRank ../inputs/ligra_wikipedia_link_en

```