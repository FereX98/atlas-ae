test=$1
if [ -z "$test" ]; then
    echo "Usage: $0 <test>"
    exit 1
fi

# disable the psf manager
sudo bash -c "echo N > /sys/kernel/debug/hermit/bks/fake_psf_manager"
cgcreate -g memory:test
cgset -r memory.limit_in_bytes=512M test

cgexec -g memory:test ${test}
ret=$?
cgdelete memory:test >/dev/null 2>&1
exit $ret
