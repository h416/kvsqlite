#!/bin/bash

CEREAL_VER="1.2.2"
CEREAL="cereal-"$CEREAL_VER
if [ ! -d $CEREAL ]; then
    curl -o $CEREAL.tar.gz -L https://github.com/USCiLab/cereal/archive/v$CEREAL_VER.tar.gz
    tar xf $CEREAL.tar.gz
fi

DLIB_VER="19.13"
DLIB="dlib-"$DLIB_VER
if [ ! -d $DLIB ]; then
    curl -o $DLIB.tar.gz -L https://github.com/davisking/dlib/archive/v$DLIB_VER.tar.gz
    tar xf $DLIB.tar.gz
fi

SQLITE="sqlite-amalgamation-3240000" 
if [ ! -d $SQLITE ]; then
    curl -O https://www.sqlite.org/2018/$SQLITE.zip
    unzip $SQLITE.zip
fi

CC=${CC:-clang}
CXX=${CXX:-clang++}

echo $CC
echo $CXX

echo "Environment: `uname -a`"
echo "Compiler: `$CXX --version`"


$CC -c -O2 -I $SQLITE -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -o sqlite.o $SQLITE/sqlite3.c
$CXX -c -O2 -std=c++11 -I include -I $CEREAL/include -I $DLIB -I $SQLITE -o test.o test.cpp
$CXX -o test test.o sqlite.o

./test
