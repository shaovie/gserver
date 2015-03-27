<?php
include_once("log.php");
include_once("recharge.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'mi_log');
$uri = $_SERVER['REQUEST_URI'];
log::instance()->debug("new con: $uri");

$pf_info = $config["mi"][stripslashes($_GET["appId"])];
if (!isset($pf_info))
{
  log::instance()->error("ret: 找不到配置");
  echo "{\"errcode\":1515}";
  exit;
}

$param = $_GET;
ksort($param);
$str = "";
foreach ($param as $key => $value)
{
  if ($key == "signature" || !isset($value))
    continue;
  if (strlen($str) == 0)
    $str = "$key=".stripslashes($value);
  else
    $str .= "&$key=".stripslashes($value);
}

$sign = hash_hmac('sha1', $str, $pf_info["AppSec"]);
if ($sign != stripslashes($_GET["signature"]))
{
  log::instance()->error("ret: 签名无效");
  echo "{\"errcode\":1525}";
  exit;
}

if (stripslashes($_GET["orderStatus"]) != "TRADE_SUCCESS")
{
  log::instance()->error("ret: 支付失败");
  echo "{\"errcode\":200}";
  exit;
}

$note = json_decode(stripslashes($_GET["cpUserInfo"]), true);
$ret = recharge (
  $pf_info["PF"],
  $note["sid"],
  stripslashes($_GET["cpOrderId"]),
  $note["uid"],
  $note["item"],
  stripslashes($_GET["payFee"]) / 100,
  stripslashes($_GET["orderId"]),
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
  echo "{\"errcode\":200}";
else if ($ret == "ACCOUNT NOT EXIST")
  echo "{\"errcode\":1516}";
else
  echo "{\"errcode\":3515,\"errMsg\":\"$ret\"}";
?>
