<?php
include_once("log.php");
include_once("recharge.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'mzw_log');
$uri = $_SERVER['REQUEST_URI'];
log::instance()->debug("new con: $uri");

$AppId = 111;
$pf_info = $config[$AppId];
$str = stripslashes($_GET["appkey"])
  .stripslashes($_GET["orderID"])
  .stripslashes($_GET["productName"])
  .stripslashes($_GET["productDesc"])
  .stripslashes($_GET["productID"])
  .stripslashes($_GET["money"])
  .stripslashes($_GET["uid"])
  .stripslashes($_GET["extern"])
  .$pf_info["AppKey"];

$sign = strtolower(md5($str));
if ($sign != stripslashes($_GET["sign"]))
{
  log::instance()->error("ret: 签名无效");
  exit;
}

// 拇指玩无法发送json格式字串，encode了一下
$note = json_decode(base64_decode(stripslashes($_GET["extern"])), true);
$ret = recharge (
  $pf_info["PF"],
  $note["zoneId"],
  $note["trade_no"],
  stripslashes($_GET["username"]),
  stripslashes($_GET["productID"]),
  stripslashes($_GET["money"]),
  stripslashes($_GET["orderID"]),
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
  echo "SUCCESS";
?>
