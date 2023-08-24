#!/bin/sh

set -x

if [ ! -d "./build" ]; then
  mkdir ./build 
fi

if [ ! -d "./logfiles" ]; then
  mkdir ./logfiles
fi

cd ./build \
  && cmake .. \
  && make