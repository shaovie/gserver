#!/bin/bash

S=`ls -d $(dirname $PWD/$0)/../s[0-9]*`
S=$(basename $S)
S=${S#*s}
proc=robotd
proc_num=6
each_payload=500
proc_flag=$USER
server_ip=127.0.0.1
server_port=$((4000 + ${S}))


run_script="run.sh"
stop_script="stop.sh"

echo -e '#!/bin/bash\n' >$run_script
echo -e '#!/bin/bash\n' >$stop_script

for ((i=1;i<=$proc_num;i++)); do
  proc_name=${proc}_${proc_flag}_$i
  echo "killall $proc_name >/dev/null 2>&1" >>$stop_script
  cd bin
  ln -sf $proc $proc_name
  cd ../
  
  file=run_$i.sh

  echo "./$file &" >>$run_script
  echo "killall $file -u $USER >/dev/null 2>&1" >>$stop_script

  echo "#!/bin/bash" > $file
  echo >> $file
  echo "export LD_LIBRARY_PATH=../libs" >> $file
  echo "while :" >> $file
  echo "do" >> $file
  echo "  killall $proc_name >/dev/null 2>&1" >> $file
  echo "  sleep 2" >> $file
  offset=`expr $i - 1`
  id_offset=`expr $each_payload \* $offset + 1`
  echo "  taskset -c 1 bin/$proc_name $server_ip $server_port $each_payload $id_offset &" >> $file
  echo "  sleep `expr $RANDOM % 600 + 3600`" >> $file
  echo "done" >> $file
  chmod +x $file
done

chmod +x $run_script
chmod +x $stop_script
