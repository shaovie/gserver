if [ "$USER" != "root" ] ; then
  echo "must root!"
  exit 1
fi

## sys luanch
chmod a+w /etc/rc.local

## time
cat > /etc/sysconfig/clock <<EOF
ZONE="Asia/Shanghai"
UTC=false
ARC=false
EOF
ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime

if ! [ -f "/usr/sbin/ntpdate" ] ; then
  echo "not found ntpdate!"
  exit 1
fi
/usr/sbin/ntpdate  tick.ucla.edu tock.gpsclock.com ntp.nasa.gov timekeeper.isi.edu usno.pa-x.dec.com &

if ! grep -q "/usr/sbin/ntpdate  tick.ucla.edu tock.gpsclock.com ntp.nasa.gov timekeeper.isi.edu usno.pa-x.dec.com" /etc/rc.local ; then
  echo "/usr/sbin/ntpdate  tick.ucla.edu tock.gpsclock.com ntp.nasa.gov timekeeper.isi.edu usno.pa-x.dec.com &" >> /etc/rc.local
fi

## modify fd limit
if ! grep -q "fs.file-max=65536" /etc/sysctl.conf ; then
  echo "fs.file-max=65536" >> /etc/sysctl.conf
  sysctl -p
fi
if ! grep -q "soft nofile" /etc/security/limits.conf ; then
  echo "*  soft nofile 8192" >> /etc/security/limits.conf
fi
if ! grep -q "hard nofile" /etc/security/limits.conf ; then
  echo "*  hard nofile 8192" >> /etc/security/limits.conf
fi

echo "ulimit -n `ulimit -n`"

## crontab
service crond restart
chkconfig --level 2345 crond on
crontab -l > crontab.yytx
if ! grep -q "/usr/sbin/ntpdate  tick.ucla.edu tock.gpsclock.com ntp.nasa.gov timekeeper.isi.edu usno.pa-x.dec.com" crontab.yytx ; then
  echo "30 0 * * * /usr/sbin/ntpdate tick.ucla.edu tock.gpsclock.com ntp.nasa.gov timekeeper.isi.edu usno.pa-x.dec.com &" >> crontab.yytx
  crontab crontab.yytx
fi
rm -f crontab.yytx

## tcp
if ! grep -q "#yytx tcp" /etc/rc.local
  echo "#yytx tcp
echo 8192 > /proc/sys/net/ipv4/tcp_max_syn_backlog
echo 40000 > /proc/sys/net/ipv4/tcp_max_tw_buckets
echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse
echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle
echo 1 > /proc/sys/net/ipv4/tcp_syncookies
echo 30 > /proc/sys/net/ipv4/tcp_fin_timeout
" >> /etc/rc.local

  echo 8192 > /proc/sys/net/ipv4/tcp_max_syn_backlog
  echo 40000 > /proc/sys/net/ipv4/tcp_max_tw_buckets
  echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse
  echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle
  echo 1 > /proc/sys/net/ipv4/tcp_syncookies
  echo 30 > /proc/sys/net/ipv4/tcp_fin_timeout
fi

## iptables
cat > /etc/sysconfig/iptables  <<EOF
# Firewall configuration written by system-config-firewall
# Manual customization of this file is not recommended.
*filter
:INPUT ACCEPT [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
-A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
-A INPUT -p icmp -j ACCEPT
-A INPUT -i lo -j ACCEPT
-A INPUT -m state --state NEW -m tcp -p tcp --dport 22 -j ACCEPT
-A INPUT -m state --state NEW -m tcp -p tcp --dport 80 -j ACCEPT
-A INPUT -j REJECT --reject-with icmp-host-prohibited
-A FORWARD -j REJECT --reject-with icmp-host-prohibited
COMMIT
EOF
chkconfig --level 3 iptables on
service iptables restart
#service iptables save

## end
sync && echo 3 > /proc/sys/vm/drop_caches && sleep 2 &&  echo 0 > /proc/sys/vm/drop_caches
