git submodule update --init --recursive
work_dir=$(dirname $(readlink -f "$0"))
cd $work_dir
rm -rf memcached >/dev/null 2>&1
mkdir memcached
cp -r ../memcached/. memcached
cp -r ../atlas_patch/*.patch memcached

sed -i 's#gitdir: ../../../.git/modules#gitdir: ../../../../.git/modules#' memcached/.git

cd memcached

patch -p1 <0001*.patch
patch -p1 <0002*.patch
patch -p1 <0003*.patch
patch -p1 <0004*.patch

./autogen.sh
./configure
make -j

if [ $? -ne 0 ]; then
    echo "make memcached failed"
    exit 1
fi

bash $work_dir/../scripts/setup_hermit.sh
