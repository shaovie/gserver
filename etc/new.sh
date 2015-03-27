S=$1
DB_PORT=3308
DB_HOST=192.168.1.220
DB_USER=han
DB_PASSWD=gh78dp
DB_NAME=game_$S
DB_PROXY_PORT=$(expr 14000 + ${S:1})
DB_PROXY_HOST=127.0.0.1
WORLD_PORT=$(expr 4000 + ${S:1})
WORLD_GM_PORT=$(expr 11000 + ${S:1})
WORLD_GM_HOST=`/sbin/ifconfig eth0 | grep "inet addr" | awk -F':|[ ]*' '{print $4}'`

self_path=$(dirname $PWD/$0)
proj_root="$self_path/.."

cd $proj_root

mkdir $S
cd $S
mkdir $S
mkdir bin

ln -sf ../etc/daily_auto_update.sh $S
ln -sf ../etc/check_core_down.sh $S
ln -sf ../etc/send_mail.py $S
cp -f ../etc/svc.json $S/
ln -sf ../etc/run.sh $S

cd bin
ln -sf ../../db_proxy/bin/db_proxy .
ln -sf ../../world/bin/iworld .
cd ../
ln -sf ../config .
ln -sf ../libs .
ln -sf ../etc .

cd $S
ln -sf ../bin/iworld .
ln -sf ../bin/db_proxy .
sed -i -e "s/DB_PORT/$DB_PORT/g" svc.json
sed -i -e "s/DB_HOST/$DB_HOST/g" svc.json
sed -i -e "s/DB_USER/$DB_USER/g" svc.json
sed -i -e "s/DB_PASSWD/$DB_PASSWD/g" svc.json
sed -i -e "s/DB_NAME/$DB_NAME/g" svc.json
sed -i -e "s/DB_PROXY_PORT/$DB_PROXY_PORT/g" svc.json
sed -i -e "s/DB_PROXY_HOST/$DB_PROXY_HOST/g" svc.json
sed -i -e "s/WORLD_PORT/$WORLD_PORT/g" svc.json
sed -i -e "s/WORLD_GM_PORT/$WORLD_GM_PORT/g" svc.json
sed -i -e "s/WORLD_GM_HOST/$WORLD_GM_HOST/g" svc.json

mkdir -p pid

mkdir cfg
cd cfg

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

cd ../../
mysql -u$DB_USER -p$DB_PASSWD -P$DB_PORT -h$DB_HOST -e "create database $DB_NAME"
mysql -u$DB_USER -p$DB_PASSWD -P$DB_PORT -h$DB_HOST $DB_NAME < etc/db.sql

