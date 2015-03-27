<?php
include_once("log.php");
include_once("recharge.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'tbt_log');
$uri = $_SERVER['REQUEST_URI'];
log::instance()->debug("new con: $uri");

$pf_info = $config["tb"][stripslashes($_GET["partner"])];
$str = "source=".stripslashes($_GET["source"])
  ."&trade_no=".stripslashes($_GET["trade_no"])
  ."&amount=".stripslashes($_GET["amount"])
  ."&partner=".stripslashes($_GET["partner"])
  ."&paydes=".stripslashes($_GET["paydes"])
  ."&debug=".stripslashes($_GET["debug"])
  ."&tborder=".stripslashes($_GET["tborder"])
  ."&key=".$pf_info["AppKey"];

$sign = strtolower(md5($str));
if ($sign != stripslashes($_GET["sign"]))
{
  log::instance()->error("ret: 签名无效");
  exit;
}

$note = json_decode(stripslashes($_GET["paydes"]), true);
$ret = recharge (
  $pf_info["PF"],
  $note["sid"],
  stripslashes($_GET["trade_no"]),
  $note["uid"],
  $note["item"],
  stripslashes($_GET["amount"]) / 100,
  stripslashes($_GET["tborder"]),
  stripslashes($_GET["debug"])
);
log::instance()->debug("ret: " . $ret);

if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
{
  $obj->status = 'success';
  echo json_encode($obj);
}
?>
