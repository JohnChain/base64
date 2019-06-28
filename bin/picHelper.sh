#!/bin/bash

FUNC=$1

if [ "x$FUNC" == "xfind" ]; then
	DESTDIR=$2
	fileList=`find . -name *.JPG`
	for name in $fileList
	do
		cp $name $DESTDIR
	done
elif [ "x$FUNC" == "xencode" ]; then
	DIRPATH=$2
	KEYWORD=$3
	DESTDIR=$4
	fileList=`find $DIRPATH | grep $KEYWORD | awk  -F\/ '{ print $NF }'`
	for name in $fileList
	do
		./base64 -e -p $DIRPATH -k $name -b 1 > $DESTDIR/$name.b64
	done
fi
