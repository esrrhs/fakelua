#! /bin/sh

NUM=53
chmod a+x fake
for i in `seq $NUM`; do
	echo "----------------running $i.fk----------------"
    ./fake $i.fk
    if [ $? -ne 0 ];then
        echo "----------------run $i.fk fail----------------"
        exit 1
    fi
    echo "----------------run $i.fk ok----------------"
done    

echo "all ok"
