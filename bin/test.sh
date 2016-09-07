#! /bin/sh

if [ $# == 1 ] && [ $1 == "-j" ];then
    JIT_FLAG=" -j "
    echo "use jit"
fi

NUM=62
chmod a+x fake
for i in `seq $NUM`; do
	echo "----------------running $i.fk----------------"
    ./fake $JIT_FLAG $i.fk
    if [ $? -ne 0 ];then
        echo "----------------run $i.fk fail----------------"
        exit 1
    fi
    echo "----------------run $i.fk ok----------------"
done    

echo "all ok"
