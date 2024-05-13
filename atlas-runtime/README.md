# Atlas

**Atlas** is an experimental project which leverages both OS and runtime techniques to improve applications' performance under the far memory settings.

## Build

1. install the [atlas kernel](https://github.com/lscat11/linux-5.14-rc5/tree/atlas)
2. clone submodules
    ```
    git submodule update --init
    ```
3. build the `remoteswap` module
    ```
    cd bks_module/remoteswap
    make -j
    ```
4. build the `bks_drv` module
    ```
    cd bks_module/bks_drv
    make -j
    ```
5. Before building, check if proper settings are enabled. E.g., card profiling (`#define CARD_PROFILING` in `lib/runtime.cc`) and to-space `deref_no_scope_slow_path` and `finish_evacuation` in `lib/pointer.cc`.
6. build the atlas runtime
    ```
    mkdir build && cd build
    cmake .. && make -j
    ```

## Run tests
Before run atlas runtime, make sure the `remoteswap client` and `remoteswap server` have been set up correctly.

1. install the `bks_drv`
    ```
    device_name=bks_dev
    drv_name=bks_drv
    bks_path=<path-to-bks_module>
    echo "Loading module $drv_name"
    sudo /sbin/rmmod ${drv_name} 2>/dev/null
    sudo /sbin/insmod "${bks_path}/bks_drv/$drv_name.ko"
    major=$(cat /proc/devices | grep "$device_name")
    major_number=($major)
    sudo rm -f /dev/${device_name} 2>/dev/null
    sudo mknod /dev/${device_name} c ${major_number[0]} 0
    major=$(cat /proc/devices | grep "$device_name")
    sudo chmod 777 /dev/${device_name}
    ```
2. disable the ASLR (Address Space Layout Randomization)
    ```
    sudo bash -c "echo 0 > /proc/sys/kernel/randomize_va_space"
    ```
3. run tests
    ```
    cd build
    make tests
    ```