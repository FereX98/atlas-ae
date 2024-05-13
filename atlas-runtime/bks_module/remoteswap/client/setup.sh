#!/bin/bash

OS_DISTRO=$( awk -F= '/^NAME/{print $2}' /etc/os-release | sed -e 's/^"//' -e 's/"$//' )

if [[ $OS_DISTRO == "CentOS Linux" ]]
then
    echo "Running on CentOS..."
    sudo chmod 777 -R /sys/fs/cgroup/cpuset
    sudo chmod 777 -R /sys/fs/cgroup/memory
    sudo chmod 777 -R /sys/kernel/mm/swap/vma_ra_enabled
elif [[ $OS_DISTRO == "Ubuntu" ]]
then
    echo "Running on Ubuntu..."
    sudo chmod 777 -R /sys/fs/cgroup/cpuset
    sudo chmod 777 -R /sys/fs/cgroup/memory
    sudo chmod 777 -R /sys/kernel/mm/swap/vma_ra_enabled
    sudo chmod 777 -R /sys/kernel/mm/swap/readahead_win
    sudo chmod 777 -R /sys/kernel/debug/
fi



# hermit
echo Y > /sys/kernel/debug/hermit/vaddr_swapout
echo Y > /sys/kernel/debug/hermit/batch_swapout
echo Y > /sys/kernel/debug/hermit/batch_io
echo Y > /sys/kernel/debug/hermit/batch_tlb
echo Y > /sys/kernel/debug/hermit/batch_account
echo Y > /sys/kernel/debug/hermit/lazy_poll
echo Y > /sys/kernel/debug/hermit/speculative_io
echo Y > /sys/kernel/debug/hermit/speculative_lock

echo N > /sys/kernel/debug/hermit/prefetch_thread
echo Y > /sys/kernel/debug/hermit/bypass_swapcache
echo 32 > /sys/kernel/mm/swap/readahead_win
echo 32 > /sys/kernel/debug/hermit/sthd_cnt

#  # baseline
#  echo N > /sys/kernel/debug/hermit/vaddr_swapout
#  echo N > /sys/kernel/debug/hermit/batch_swapout
#  echo N > /sys/kernel/debug/hermit/swap_thread
#  echo N > /sys/kernel/debug/hermit/bypass_swapcache
#  echo N > /sys/kernel/debug/hermit/speculative_io
#  echo N > /sys/kernel/debug/hermit/speculative_lock
#  echo N > /sys/kernel/debug/hermit/prefetch_thread


#sudo swapoff /dev/nvme0n1p3
