#!/bin/env bash

#-----fetch here!!
svn_user="--username=bpublish --password=78798324545"

cd $(dirname $PWD/$0)
self_path=`pwd`
proj_root="$self_path/../.."
log_string=""
update_state="update ok"
cfg_file="svc.json"

function revision_change()
{
  cd $proj_root
  old_revision=`svn info | grep '^Last Changed Rev' | awk '{print $4}'`
  svn update $svn_user --non-interactive --no-auth-cache -q >/dev/null 2>&1
  new_revision=`svn info | grep '^Last Changed Rev' | awk '{print $4}'`
  [ $new_revision == $old_revision ] && return 0

  return 1
}
function compile()
{
  cd ${proj_root}/$1
  make cleanall >/dev/null 2>&1
  make platform=-DFOR_IROBOT >/dev/null 2>make_$1_err_log
  if [ $? != 0 ]; then
    log_string="${log_string}make $1 failed\n`cat make_$1_err_log | sed 's/^/  &/g'`\n"
    update_state="update failed"
  else
    log_string="${log_string}make $1 ok\n"
  fi

  [ -f make_$1_err_log ] && rm make_$1_err_log
}
function stop_robot()
{
  cd $proj_root/robot
  sh ./stop.sh
}
function start_robot()
{
  cd $proj_root/robot
  sh ./init_run.sh
  sh ./run.sh
  sleep 5
  if [ -z "`ps -ef | grep robotd_$USER | grep -v grep`" ]; then
    log_string="$log_string""robot launch failed\n"
    update_state="update failed"
  else
    log_string="$log_string""robot launch ok\n"
  fi
}
function stop_server()
{
  cd $self_path
  pid=`ps -ef | grep check_core_down | egrep -v "(grep|vim)" | awk '{if($1==$UID);print $2}'`
  [ ! -z "$pid" ] && kill $pid >/dev/null 2>&1
  sh ./run.sh stop all
}
function start_server()
{
  cd $self_path
  sh ./run.sh start all
  sleep 3
  platform=`awk -F '"|:|[ ]*' '/platform/{print $7}' $cfg_file`
  [ -z "$platform" ] && platform="qq"
  for s in db_proxy iworld; do
    if [ -z "`ps -ef | grep $USER | grep $s | grep $platform`" ]; then
      log_string="${log_string}$s launch failed\n"
      update_state="update failed"
    else
      log_string="${log_string}$s launch ok\n"
    fi
  done

  #only check core down when server launch ok
  [ "$update_state" == "update ok" ] && sh check_core_down.sh &
}
function rebuild_db()
{
  cd $self_path
  db_port=`grep -A6 '"db"' $cfg_file | awk -F ':|"|[ ]*|,' '/"port"/{print $6}'`
  db_host=`grep -A6 '"db"' $cfg_file | awk -F ':|"|[ ]*|,' '/"host"/{print $7}'`
  db_user=`grep -A6 '"db"' $cfg_file | awk -F ':|"|[ ]*|,' '/"user"/{print $7}'`
  db_passwd=`grep -A6 '"db"' $cfg_file | awk -F ':|"|[ ]*|,' '/"passwd"/{print $7}'`
  db_name=`grep -A6 '"db"' $cfg_file | awk -F ':|"|[ ]*|,' '/"db_name"/{print $7}'`

  cd $proj_root
  mysql -u$db_user -p$db_passwd -P$db_port -h$db_host -e "create database $db_name"
  mysql -u$db_user -p$db_passwd -P$db_port -h$db_host $db_name < etc/db.sql
}

revision_change
if [ $? == 1 ]; then
  for s in world db_proxy robot; do 
    compile $s
  done
  stop_robot
  stop_server
  rebuild_db
  start_server
  start_robot
else
  log_string="${log_string}revision not changed, dont need update\n"
fi

# send mail
cd $self_path
python send_mail.py "gserver_${self_path##*/}" "$update_state" "$log_string"
