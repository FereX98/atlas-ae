git submodule update --init --recursive
work_dir=$(dirname $(readlink -f "$0"))
cd $work_dir
rm -rf phoenix > /dev/null 2>&1
cp -r ../phoenix .
#sed -i 's#gitdir: ../../../.git/modules#gitdir: ../../../../.git/modules#' phoenix/.git
cp -r ../*.patch phoenix
cd phoenix
patch -p1 < 0001*.patch
cd phoenix-2.0
# Shi: multi-threaded compilation may cause libphoenix.a to be used before it is generated???
make clean && make
# echo "Please make sure the jemalloc has been installed!"
#bash $work_dir/../scripts/setup_hermit.sh
#echo 1 > /sys/kernel/debug/hermit/sthd_cnt
# bash $work_dir/../scripts/set_cpufreq.sh
sudo chmod -R 777 /sys/fs/cgroup/memory