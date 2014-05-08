#!/bin/bash

LIB_DIR='external-libs' 

LIB_NAMES=('lufa' 'wallaroo')
LIB_URLS=('https://github.com/abcminiuser/lufa.git' 'https://code.google.com/p/wallaroo/')

mkdir -p $LIB_DIR
cd $LIB_DIR

for i in ${!LIB_NAMES[*]}
do
	echo '* Library' ${LIB_NAMES[$i]}
	if [ -d ${LIB_NAMES[$i]} ]
	then
		echo 'Pulling latest version...'
		cd ${LIB_NAMES[$i]} && git pull && cd -
	else
		echo 'Cloning repository:' ${LIB_URLS[$i]}
		git clone ${LIB_URLS[$i]} ${LIB_NAMES[$i]}
	fi
done

