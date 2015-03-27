#!/bin/bash

export LD_LIBRARY_PATH=../../libs

if [ ! -f svc_test ]; then
  make
fi

./svc_test 192.168.1.221 6001
