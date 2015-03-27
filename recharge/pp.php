<?php
include_once("log.php");
include_once("recharge.php");
include_once("ssl.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'pp_log');
$uri = $_SERVER['REQUEST_URI'];
$body = file_get_contents('php://input');
log::instance()->debug("new con: $uri $body");

$pf_info = $config["pp"][stripslashes($_POST["app_id"])];
$sign = base64_decode(stripslashes($_POST["sign"]));

$source = json_decode(publickey_decodeing($sign, $pf_info["PubKey"]), true);
if (empty($source)
  || $source["order_id"] != stripslashes($_POST["order_id"])
  || $source["billno"] != stripslashes($_POST["billno"])
  || $source["account"] != stripslashes($_POST["account"])
  || $source["amount"] != stripslashes($_POST["amount"])
  || $source["status"] != stripslashes($_POST["status"])
  || $source["app_id"] != stripslashes($_POST["app_id"])
  || $source["uuid"] != stripslashes($_POST["uuid"])
  || $source["roleid"] != stripslashes($_POST["roleid"])
  || $source["zone"] != stripslashes($_POST["zone"]))
{
  log::instance()->error("ret: 签名无效");
  echo "fail";
  exit;
}
if ($source["status"] == 1)
{
  log::instance()->error("ret: status 为已兑换过并成功返回");
  echo "success";
  exit;
}

$note = json_decode(base64_decode(stripslashes($_POST["roleid"])), true);
$ret = recharge (
  $pf_info["PF"],
  stripslashes($_POST["zone"]),
  stripslashes($_POST["billno"]),
  $note["uid"],
  $note["item"],
  stripslashes($_POST["amount"]),
  stripslashes($_POST["order_id"]),
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
  echo "success";
else
  echo "fail";
?>
