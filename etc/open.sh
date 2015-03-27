#######################################################

#usage: sh open.sh s1

S=$1
GAME=mszm

SN=${S:1}
PLATFORM=rc

DB_PORT=3306
DB_HOST=10.66.101.109
DB_USER=root
DB_PASSWD=cloudia@cn
DB_NAME=${GAME}_${S}
SLAVE_DB_PORT=3306
SLAVE_DB_HOST=10.66.101.109
SLAVE_DB_USER=slaveroot
SLAVE_DB_PASSWD=cloudia@cn

DB_PROXY_PORT=$(expr 14000 + ${S:1})
DB_PROXY_HOST=127.0.0.1
WORLD_PORT=$(expr 4000 + ${S:1})
WORLD_GM_PORT=$(expr 11000 + ${S:1})
WORLD_GM_HOST=127.0.0.1

CROSS_PORT=1023
CROSS_HOST=192.168.1.229

cd $(dirname $PWD/$0)/../

./mysql/bin/mysql -u$DB_USER -p$DB_PASSWD -P$DB_PORT -h$DB_HOST -e "create database $DB_NAME"
./mysql/bin/mysql -u$DB_USER -p$DB_PASSWD -P$DB_PORT -h$DB_HOST $DB_NAME < etc/db.sql
echo "./mysql/bin/mysql -u$DB_USER -p$DB_PASSWD -P$DB_PORT -h$DB_HOST $DB_NAME" > db.$S

[ -d "$S" ] || mkdir $S && cd $S

ln -sf ../etc/run.sh .
ln -sf ../bin/iworld .
ln -sf ../bin/db_proxy .

cat > svc.json << EOF
{
  "platform": "$PLATFORM",
  "cfg_root": "../config",
  "group": "test",
  "sn": $SN,
  "max_online": 1500,

  "cross": {
    "port": $CROSS_PORT,
    "host": "$CROSS_HOST"
  },
  "db": {  
    "port": $DB_PORT,
    "host": "$DB_HOST",
    "user": "$DB_USER",
    "passwd": "$DB_PASSWD",
    "db_name": "$DB_NAME"
  },
  "slave": {
    "port": $SLAVE_DB_PORT,
    "host": "$SLAVE_DB_HOST",
    "user": "$SLAVE_DB_USER",
    "passwd": "$SLAVE_DB_PASSWD",
    "db_name": "$DB_NAME"
  },
  "db_proxy": {
    "port": $DB_PROXY_PORT,
    "host": "$DB_PROXY_HOST"
  },
  "world": {
    "port": $WORLD_PORT,
    "gm_port": $WORLD_GM_PORT,
    "gm_host": "$WORLD_GM_HOST"
  }
}
EOF

[ -d "cfg" ] || mkdir cfg && cd cfg

for s in "iworld" "db_proxy"
do
  echo "base=ALLS
file_name=$s.err
dir=log
roller=5
file_size=20M" > err_log.$s

  echo "base=ALLS
file_name=$s.sys
dir=log
roller=5
file_size=20M" > sys_log.$s

  echo "base=ALLS
file_name=$s.imp
dir=log
roller=5
file_size=20M" > imp_log.$s

done

crontab -l > /tmp/cron_tmp.result
s=`grep "move_log.sh $S" /tmp/cron_tmp.result`
if [ "$s" == "" ]; then
  echo "* * * * * cd /data/yytx/shell && ./move_log.sh $S" >> /tmp/cron_tmp.result
  crontab /tmp/cron_tmp.result
fi
rm -f /tmp/cron_tmp.result

