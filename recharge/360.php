<?php
include_once("log.php");
include_once("recharge.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', '360_log');
$uri = $_SERVER['REQUEST_URI'];
log::instance()->debug("new con: $uri");

$AppId = "201932111";
$pf_info = $config["360"][$AppId];

$param = $_GET;
ksort($param);
$str = "";
foreach ($param as $key => $value)
  if ($key != "sign" && $key != "sign_return")
    $str = $str.stripslashes($value)."#";
$str = $str.$pf_info["AppSec"];

$sign = strtolower(md5($str));
if ($sign != stripslashes($_GET["sign"]))
{
  log::instance()->error("ret: 签名无效");
  exit;
}
echo "ok";

if (stripslashes($_GET["gateway_flag"]) != "success")
{
  log::instance()->error("ret: 支付失败");
  exit;
}

$note = json_decode(stripslashes($_GET["app_ext1"]), true);
$ret = recharge (
  $pf_info["PF"],
  $note["sid"],
  stripslashes($_GET["app_order_id"]),
  $note["uid"],
  $note["item"],
  stripslashes($_GET["amount"]) / 100,
  stripslashes($_GET["order_id"]),
  0
);
log::instance()->debug("ret: " . $ret);
?>
