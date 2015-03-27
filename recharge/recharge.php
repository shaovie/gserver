<?php
// $pay : 以 元 为单位
function recharge
  ($platform, $zoneid, $trade_no, $account, $rc_type, $pay, $orderid, $debug)
{
  $servers = include 'servers.php';

  $target_ip = $servers[$platform]['s' . $zoneid];
  if (empty($target_ip))
    return "Zone ID ERROR";

  $port = 80;

  @$socket = fsockopen($target_ip, $port, $errno, $errstr, 10);
  if (!$socket)
    return "socket remote open failed!";

  $uri = "/transpond.php"
    ."?cmd=".urlencode("platform_recharge")
    ."&zoneid=".urlencode($zoneid)
    ."&trade_no=".urlencode($trade_no)
    ."&account=".urlencode($account)
    ."&rc_type=".urlencode($rc_type)
    ."&pay=".urlencode($pay)
    ."&platform=".urlencode($platform)
    ."&orderid=".urlencode($orderid)
    ."&debug=".urlencode($debug);
  $out = "GET $uri HTTP/1.0\r\n\r\n";
  fwrite($socket, $out);

  while (!feof($socket))
    $ret = fgets($socket, 5120);

  fclose($socket);
  return $ret;
}
?>
