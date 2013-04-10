#!/bin/sh

mkdir objs
for f in `ls -1 maps/*.bsp`
do
	fn=$(basename "$f")
	prefix=`echo $fn | awk -F'.' '{ print $1 }'`
	Debug/bsp2obj.exe "$f" "objs/$prefix.obj"
	if [ $? -ne 0 ]
	then
		exit $?
	fi
done
