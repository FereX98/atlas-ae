git submodule update --init --recursive
work_dir=$(dirname $(readlink -f "$0"))
cd $work_dir
rm -rf gapbs > /dev/null 2>&1
cp -r ../gapbs .
cp -r ../*.patch gapbs
cd gapbs
patch -p1 < 000*.patch
make tc
make pr
sudo chmod -R 777 /sys/fs/cgroup/memory
bash $work_dir/../scripts/setup_atlas.sh
