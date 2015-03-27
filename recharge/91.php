<?php
include_once("log.php");
include_once("recharge.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', '91_log');
$uri = $_SERVER['REQUEST_URI'];
log::instance()->debug("new con: $uri");

$pf_info = $config["91"][stripslashes($_GET["AppId"])];
$str = stripslashes($_GET["AppId"])
  .stripslashes($_GET["Act"])
  .stripslashes($_GET["ProductName"])
  .stripslashes($_GET["ConsumeStreamId"])
  .stripslashes($_GET["CooOrderSerial"])
  .stripslashes($_GET["Uin"])
  .stripslashes($_GET["GoodsId"])
  .stripslashes($_GET["GoodsInfo"])
  .stripslashes($_GET["GoodsCount"])
  .stripslashes($_GET["OriginalMoney"])
  .stripslashes($_GET["OrderMoney"])
  .stripslashes($_GET["Note"])
  .stripslashes($_GET["PayStatus"])
  .stripslashes($_GET["CreateTime"])
  .$pf_info["AppKey"];

$sign = strtolower(md5($str));
if ($sign != stripslashes($_GET["Sign"]))
{
  log::instance()->error("ret: 签名无效");
  $obj->ErrorCode = 5;
  $obj->ErrorDesc = "签名无效";
  echo json_encode($obj);
  exit;
}

if (stripslashes($_GET["PayStatus"]) == 0)
{
  log::instance()->error("ret: 支付失败");
  $obj->ErrorCode = 1;
  $obj->ErrorDesc = "为什么失败还请求";
  echo json_encode($obj);
  exit;
}

$note = json_decode(stripslashes($_GET["GoodsId"]), true);
$ret = recharge (
  $pf_info["PF"],
  $note["sid"],
  stripslashes($_GET["CooOrderSerial"]),
  $note["uid"],
  $note["item"],
  stripslashes($_GET["OrderMoney"]),
  stripslashes($_GET["ConsumeStreamId"]),
  0
);
log::instance()->debug("ret: " . $ret);

$obj->ErrorCode = 4;
$obj->ErrorDesc = $ret;
if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
  $obj->ErrorCode = 1;

echo json_encode($obj);
?>
