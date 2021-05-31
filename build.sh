#!/usr/bin/env bash

CPUCOUNT=$(grep -c ^processor /proc/cpuinfo)

BUILD_CPPFLAGS="-I/usr/local/include/clio"
BUILD_CXXFLAGS=""
BUILD_LDFLAGS=""

# check for DEBUG or RELEASE passed in
if [ "$#" -gt 0 ]; then
	if [ "$1" = "DEBUG" ]; then
		BUILD_CPPFLAGS+=" -DDEBUG"
		BUILD_CXXFLAGS+=" -g -O0"
	elif [ "$1" = "RELEASE" ]; then
		BUILD_CPPFLAGS+=" -DNDEBUG"
		BUILD_CXXFLAGS+=" -O2"
	fi
fi

# build project
autoreconf -vfi
./configure CPPFLAGS="${CPPFLAGS} ${BUILD_CPPFLAGS}" CXXFLAGS="${CXXFLAGS} ${BUILD_CXXFLAGS}" LDFLAGS="${LDFLAGS} ${BUILD_LDFLAGS}"
make clean
make -j$CPUCOUNT

