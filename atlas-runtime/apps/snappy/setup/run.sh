#!/bin/bash
work_dir=$(dirname $(readlink -f "$0"))
cd ${work_dir}

# Download ucompressed input.
wget http://cs.fit.edu/~mmahoney/compression/enwik9.zip
unzip enwik9.zip
mv enwik9 enwik9.uncompressed

# Create compressed input.
make clean
make
./main
