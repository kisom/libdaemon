#!/bin/sh
# simple shell script to read a manifest file and copy the files in the
# manifest to another dir, i.e. to build a distribution

if [ "$1" -a "$2" ]; then
        MANIFEST=$1
        DESTDIR=$2
else
        echo "usage: distcp.sh MANIFEST DESTDIR"
        echo "    MANIFEST should be a file containing files to DESTDIR"
        echo ""
        exit 1
fi

FILELIST=$(cat $MANIFEST | xargs)

if [ -z "${FILELIST}" ]; then
        echo "MANIFEST cannot be empty!"
        exit 1
fi

for file in ${FILELIST} 
do
    if [ ! -d ${DESTDIR}/$(dirname ${file}) ]; then
        echo "creating ${DESTDIR}/$(dirname ${file})"
        mkdir -p ${DESTDIR}/$(dirname ${file})
    fi

    cp -ruv -t ${DESTDIR}/$(dirname ${file})/ $file
done 


