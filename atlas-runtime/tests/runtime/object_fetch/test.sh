test=$1
if [ -z "$test" ]; then
    echo "Usage: $0 <test>"
    exit 1
fi

cgcreate -g memory:object_fetch
cgset -r memory.limit_in_bytes=512M object_fetch

cgexec -g memory:object_fetch ${test}
ret=$?
cgdelete memory:object_fetch >/dev/null 2>&1
exit $ret
