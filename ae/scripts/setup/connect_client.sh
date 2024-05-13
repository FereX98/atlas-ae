#!/bin/bash

sudo service opensm start

echo "Installing remoteswap client"
cd /home/osdi/remoteswap/client
bash manage_rswap_client.sh install

echo "Installing block device"
atlas_dir=/home/osdi/mcd/Paris
echo "Creating bks device"
cd $atlas_dir/scripts
device_name=bks_dev
drv_name=bks_drv
bks_path=../bks_module
echo "Loading module $drv_name"
sudo /sbin/rmmod ${drv_name} 2>/dev/null
sudo /sbin/insmod "${bks_path}/bks_drv/$drv_name.ko"
major=$(cat /proc/devices | grep "$device_name")
major_number=($major)
sudo rm -f /dev/${device_name} 2>/dev/null
sudo mknod /dev/${device_name} c ${major_number[0]} 0
major=$(cat /proc/devices | grep "$device_name")
sudo chmod 777 /dev/${device_name}

echo "Setting up global variables"
sudo bash -c "echo 0 > /proc/sys/kernel/randomize_va_space"
# Disable hyperboost as it may cause performance variations
echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo
sudo chmod 777 -R /sys/fs/cgroup/cpuset
sudo chmod 777 -R /sys/fs/cgroup/memory
sudo chmod 777 -R /sys/kernel/mm/swap/vma_ra_enabled
sudo chmod 777 -R /sys/kernel/mm/swap/readahead_win
sudo chmod 777 -R /sys/kernel/debug/