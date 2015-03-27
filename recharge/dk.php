<?php
include_once("log.php");
include_once("recharge.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'dk_log');
$uri = $_SERVER['REQUEST_URI'];
log::instance()->debug("new con: $uri");

$AppId = "3491827";
$pf_info = $config["dk"][$AppId];
$str = stripslashes($_GET["amount"])
  .stripslashes($_GET["cardtype"])
  .stripslashes($_GET["orderid"])
  .stripslashes($_GET["result"])
  .stripslashes($_GET["timetamp"])
  .$pf_info["AppKey"]
  .urlencode(stripslashes($_GET["aid"]));

$sign = strtolower(md5($str));
if ($sign != stripslashes($_GET["client_secret"]))
{
  log::instance()->error("ret: 签名无效");
  echo "ERROR_SIGN";
  exit;
}

if (stripslashes($_GET["result"]) == 2)
{
  log::instance()->error("ret: 支付失败");
  echo "SUCCESS";
  exit;
}

$note = json_decode(stripslashes($_GET["aid"]), true);
$ret = recharge (
  $pf_info["PF"],
  $note["sid"],
  stripslashes($_GET["orderid"]),
  $note["uid"],
  $note["item"],
  stripslashes($_GET["amount"]),
  stripslashes($_GET["orderid"]),
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret == "TRADE_NO NOT EXIST")
  $ret = "ERROR_REPEAT";
else if ($ret != "SUCCESS")
  $ret = "ERROR_FAIL";

echo $ret;
?>
