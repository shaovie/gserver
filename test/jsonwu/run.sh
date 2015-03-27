#!/bin/bash

if [ ! -x "./mytest" ];then
  make
fi

./mytest -t 3 -c 100 -n 1000 &
