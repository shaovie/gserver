#!/bin/bash

opts='[start|stop|restart]'
svcs='[all|iworld|db_proxy]'

if [ $# -ne 2 ]; then
  echo -e "Usage:\n  $0 $opts $svcs";
  exit;
fi

function start_svc()
{
  if [ "$1" == "db_proxy" ]; then
    ./$1 -g -u $game_user -C -p $platform -s $sn -n 17 &
  elif [ "$1" == "iworld" ]; then
    taskset -c 1 ./$1 -g -u $game_user -C -p $platform -s $sn &
  fi
}
function stop_svc()
{
  file="pid/$1.pid"
  if [ -f $file ]; then
    pid=`cat $file`
    if [ -d "/proc/$pid" ]; then
      kill -HUP $pid;
    fi
  fi
}
function start_all() 
{ 
  start_svc db_proxy; sleep 1; start_svc iworld 
}
function stop_all() 
{ 
  stop_svc iworld; sleep 2; stop_svc db_proxy 
}

opt=$1
svc=$2

if [ "$opt" != "start" -a "$opt" != "stop" -a "$opt" != "restart" ]; then
  echo "opt arg err, options:$opts"; exit
fi
if [ "$svc" != "all" -a "$svc" != "iworld" -a "$svc" != "db_proxy" ]; then
  echo "svc arg err, options:$svcs"; exit
fi

cd $(dirname $PWD/$0)
game_user="$USER"
platform=`awk -F ":|," '/platform/{print $2}' svc.json | sed 's/"//g'`
sn=`awk -F ":|," '/sn/{print $2}' svc.json`

export LD_LIBRARY_PATH=../libs

if [ "$opt" == "start" ]; then
  if [ "$svc" == "all" ]; then
    start_all
  else
    start_svc $svc
  fi
elif [ "$opt" == "restart" ]; then
  if [ "$svc" == "all" ]; then
    stop_all; sleep 2; start_all
  else
    stop_svc $svc; sleep 2; start_svc $svc
  fi
else
  if [ "$svc" == "all" ]; then
    stop_all
  else
    stop_svc $svc
  fi
fi
