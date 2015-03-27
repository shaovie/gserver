echo 'module("msg")' > src/msg.lua
echo "" >> src/msg.lua
awk '/define/{if ($3 > 1000) {print $2,"=",$3;}}' ../common/message.h >> src/msg.lua

