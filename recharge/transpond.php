<?php
header("Content-type: text/html; charset=utf-8");

$zoneid = $_GET["zoneid"];
$cmd = $_GET["cmd"];

if (!isset($zoneid) || !isset($cmd))
  exit("请求无效");

// 没有 key 进行验证，所以目前只支持 platform_recharge
if ($cmd != "platform_recharge")
  exit("不支持的命令");

$target_ip = "127.0.0.1";
$port = 11000 + $zoneid;

@$socket = fsockopen($target_ip, $port, $errno, $errstr, 10);
if (!$socket)
  exit("socket local open failed!");

$uri = "/$cmd?from=self";
foreach ($_GET as $key => $value)
  if ($key != "zoneid" && $key != "cmd")
    $uri = "$uri&$key=" . urlencode($value);

$out = "GET $uri HTTP/1.0\r\n\r\n";
fwrite($socket, $out);

while (!feof($socket))
  $ret = fgets($socket, 5120);

fclose($socket);
echo $ret;
?>
