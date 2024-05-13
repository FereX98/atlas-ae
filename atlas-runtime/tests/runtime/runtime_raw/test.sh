test=$1
if [ -z "$test" ]; then
    echo "Usage: $0 <test>"
    exit 1
fi

cgcreate -g memory:runtime_raw
cgset -r memory.limit_in_bytes=512M runtime_raw

cgexec -g memory:runtime_raw ${test}
ret=$?
cgdelete memory:runtime_raw >/dev/null 2>&1
exit $ret
