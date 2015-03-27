#!/bin/bash

cd $(dirname $PWD/$0)
self_path=`pwd`

world_ppid=`cat pid/iworld.pid`
db_ppid=`cat pid/db_proxy.pid`

world_pid=`pgrep -P $world_ppid`
db_pid=`pgrep -P $db_ppid`

while [ 1 ]
do
  log_string=""
  [ "`pgrep -P $world_ppid`x" != "${world_pid}x" ] && log_string="${log_string}world core down\n"
  [ "`pgrep -P $db_ppid`x" != "${db_pid}x" ] && log_string="${log_string}db_proxy core down\n"

  if [ -z "$log_string" ]; then
    sleep 10
  else
    python send_mail.py "gserver_${self_path##*/}" "core down" "$log_string"
    break
  fi
done
