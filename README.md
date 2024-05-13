# Artifact Evaluation Instructions for Atlas

This repo includes Atlas kernel and runtime, as well as a set of scripts to reproduce major results in the paper (figure 4, 5 and 6).

## Accessing the evaluation environment

To eliminate the complicated setup process, we have set up an environment to run all Atlas numbers. Please leave your public key as a comment on HotCRP to let us give your access to the servers. After your public key is added, you should be able to access the CPU server, memory server, as well as the jump server, listed on the HotCRP website.

Please coordinate with other reviewers to ensure that only one reviewer is conducting experiments on the servers at a time. Otherwise, the scripts may not run as expected.

Due to limited hardware resources, we currently do not have enough servers with the exact hardware configuration to set up the environment of baseline systems. Therefore, we provide pre-run baseline results on the evaluation environment to draw figures. Please contact us if you are interested in running the baseline systems. We will set up the environment once we have access to more servers.

## File structure on the evaluation environment

* `ae`: AE scripts and data
  * `scripts`: scripts to run experiments and draw figures
  * `results`: Atlas results you run will be stored here
  * `results_baselines`: Pre-run AIFM and Fastswap results to draw figures
  * `README.md`: This instruction
* `remoteswap`: RDMA library to connect the CPU and memory servers
* `atc`, `df`, `gpr`, `mcd`, `mpvc`, `mwc`, `ws`: Individual self-contained applications

## Setting up RDMA connection

**This is only required after server reboot.** Run `ps aux | grep rswap` on 76 to see whether remoteswap server is already running. If so, skip this step.

```bash
# on memory server. Wait until it returns to ensure server is listening for connections
bash /home/osdi/ae/scripts/setup/connect_server.sh
# on cpu server
bash /home/osdi/ae/scripts/setup/connect_client.sh
```

**After setting up, all scripts run on the CPU server.**

## Kick-the-tires

To ensure that the artifact is working, we recommend reproducing figure 6b, which tests the connection between two servers as well as the Memcached client.

```bash
# This produces Atlas latency CDF data at /home/osdi/ae/results/fig6b/atlas.csv
bash /home/osdi/ae/scripts/fig6/cdf.sh
# This generate figure is at /home/osdi/ae/scripts/drawing/fig6b.pdf
python3 /home/osdi/ae/scripts/drawing/fig6b.py
```

## Full evaluation process

### Figure 4

To run an application, use the scripts provided in `/home/osdi/ae/scripts/fig4/`. Each script produces Atlas performance numbers in the corresponding folder under `/home/osdi/ae/results/fig4`.

```bash
# run all experiments
bash /home/osdi/ae/scripts/fig4/run-all.sh
# run individual experiments
bash /home/osdi/ae/scripts/fig4/mcd-cl.sh
bash /home/osdi/ae/scripts/fig4/mcd-u.sh
bash /home/osdi/ae/scripts/fig4/gpr.sh
bash /home/osdi/ae/scripts/fig4/atc.sh
bash /home/osdi/ae/scripts/fig4/df.sh
bash /home/osdi/ae/scripts/fig4/ws.sh
bash /home/osdi/ae/scripts/fig4/mwc.sh
bash /home/osdi/ae/scripts/fig4/mpvc.sh
```

After collecting all data, generate the figure:

```bash
# This generate figure is at /home/osdi/ae/scripts/drawing/fig4.pdf
python3 /home/osdi/ae/scripts/drawing/fig4.py
```

### Figure 5a

```bash
# This produces Atlas latency-throughput data under /home/osdi/ae/results/fig5a
bash /home/osdi/ae/scripts/fig5/ltc.sh
# This generate figure is at /home/osdi/ae/scripts/drawing/fig5a.pdf
python3 /home/osdi/ae/scripts/drawing/fig5a.py
```

### Figure 5b

```bash
# This produces Atlas latency CDF data at /home/osdi/ae/results/fig5b/atlas.csv
bash /home/osdi/ae/scripts/fig5/cdf.sh
# This generate figure is at /home/osdi/ae/scripts/drawing/fig5b.pdf
python3 /home/osdi/ae/scripts/drawing/fig5b.py
```

### Figure 6a

```bash
# This produces Atlas latency-throughput data under /home/osdi/ae/results/fig6a
bash /home/osdi/ae/scripts/fig6/ltc.sh
# This generate figure is at /home/osdi/ae/scripts/drawing/fig6a.pdf
python3 /home/osdi/ae/scripts/drawing/fig6a.py
```

### Figure 6b

See kick-the-tires instructions.

## Building and running

This section is only used when building and running Atlas on your own servers. Please do not try this process on our provided evaluation servers as it may break the already setup environment.

### Building Atlas kernel

```bash
cd linux-5.14-rc5
cp config .config
sudo apt install -y build-essential bc python2 bison flex libelf-dev libssl-dev libncurses-dev libncurses5-dev libncursesw5-dev
./build_kernel.sh build
./build_kernel.sh install
./build_kernel.sh headers-install
## edit GRUB_DEFAULT="Advanced options for Ubuntu>Ubuntu, with Linux 5.14.0-rc5+", or what ever the new kernel version code it
## GRUB_CMDLINE_LINUX="nokaslr transparent_hugepage=never processor.max_cstate=0 intel_idle.max_cstate=0 tsx=on tsx_async_abort=off mitigations=off quiet splash noibrs noibpb nospec_store_bypass_disable no_stf_barrier"
sudo vim /etc/default/grub
sudo update-grub
# ensure the right version of kernel is booted and the correct boot options are used. The command below should have output
sudo reboot

# Install the correct version of MLNX_OFED driver based on your OS version
wget https://content.mellanox.com/ofed/MLNX_OFED-5.5-1.0.3.2/MLNX_OFED_LINUX-5.5-1.0.3.2-ubuntu18.04-x86_64.tgz
wget https://content.mellanox.com/ofed/MLNX_OFED-5.5-1.0.3.2/MLNX_OFED_LINUX-5.5-1.0.3.2-ubuntu20.04-x86_64.tgz
wget https://content.mellanox.com/ofed/MLNX_OFED-5.6-1.0.3.3/MLNX_OFED_LINUX-5.6-1.0.3.3-ubuntu22.04-x86_64.tgz
# Use Ubuntu 18.04 as an example below
tar xzf MLNX_OFED_LINUX-5.5-1.0.3.2-ubuntu18.04-x86_64.tgz
cd MLNX_OFED_LINUX-5.5-1.0.3.2-ubuntu18.04-x86_64
sudo apt install -y bzip2
sudo ./mlnxofedinstall --add-kernel-support

sudo /etc/init.d/openibd restart

sudo update-rc.d opensmd remove -f
sudo sed "s/# Default-Start: null/# Default-Start: 2 3 4 5/g" /etc/init.d/opensmd -i
sudo systemctl enable opensmd
# before reboot, may need to manually enable the service
sudo service opensmd start
# check the status of the service, should be active
service opensmd status

# get device name, mlx5_2 here 
ibstat
# LINK_TYPE_P1=1 - IB LINK_TYPE_P1=2 - Ethernet
sudo mstconfig -d mlx5_2 set LINK_TYPE_P1=2
sudo mstfwreset -d mlx5_2 -l3 -y reset
ibstat mlx5_2
# Maybe need to manually bring up the IB interface
sudo ifconfig ibs5f0 up
# The IP of the InfiniBand interface may need to be manually set up
sudo vim /etc/netplan/*.yaml
sudo netplan apply
# or with network manager
sudo nmtui
```

### Building Atlas runtime

```bash
cd atlas-runtime
cd third_party
git clone --depth 1 -b 54eaed1d8b56b1aa528be3bdd1877e59c56fa90c https://github.com/jemalloc/jemalloc.git
# on memory server
cd bks_module/remoteswap/server
make
# on CPU server
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt install g++-9
sudo apt install cgroup-tools
cd bks_module/remoteswap/client
make -j
# compile kernel module
cd ../../bks_drv
make -j
# compile main program
cd ../..
mkdir build
cd build
# Use gcc-9
sudo update-alternatives --config gcc
cmake ..
make -j

cd Paris/bks_module/remoteswap/server
#./rswap-server <memory server ip> <memory server port> <memory pool size in GB> <number of cores on CPU server>
./rswap-server 172.16.16.1 9999 48 96

cd Paris/bks_module/remoteswap/client
## edit accordingly
bash manage_rswap_client.sh install

cd ../../../scripts
device_name=bks_dev
drv_name=bks_drv
bks_path=../bks_module
sudo /sbin/rmmod ${drv_name} 2>/dev/null
sudo /sbin/insmod "${bks_path}/bks_drv/$drv_name.ko"
major=$(cat /proc/devices | grep "$device_name")
major_number=($major)
sudo rm -f /dev/${device_name} 2>/dev/null
sudo mknod /dev/${device_name} c ${major_number[0]} 0
major=$(cat /proc/devices | grep "$device_name")
sudo chmod 777 /dev/${device_name}
sudo bash -c "echo 0 > /proc/sys/kernel/randomize_va_space"
echo 0 | sudo tee /sys/devices/system/cpu/cpufreq/boost
```