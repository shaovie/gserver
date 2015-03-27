# 系统存在大量TIME_WAIT状态的连接，通过调整内核参数解决
net.ipv4.tcp_syncookies = 1
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_tw_recycle = 1
net.ipv4.tcp_fin_timeout = 30

sysctl -p
# 
netstat -n | awk '/^tcp/ {++S[$NF]} END {for(a in S) print a, S[a]}'


0 * * * *  find /data0/htdocs/localhost/audio -type f -mtime +2 -exec rm {} \;
