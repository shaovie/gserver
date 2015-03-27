<?php
include_once("log.php");
include_once("recharge.php");
include_once("ssl.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'itools_log');
$uri = $_SERVER['REQUEST_URI'];
$body = file_get_contents('php://input');
log::instance()->debug("new con: $uri $body");

$AppID = "533";
$pf_info = $config["itools"][$AppID];

// RSA verify
$notify_data = base64_decode(stripslashes($_POST["notify_data"]));
$notify_data = publickey_decodeing_sectionalized($notify_data, 128, $pf_info["PubKey"]);
$sign = base64_decode(stripslashes($_POST["sign"]));

if (!rsa_verify($notify_data, $sign, $pf_info["PubKey"]))
{
  log::instance()->error("ret: 签名无效");
  echo "fail";
  exit;
}
// end verify
echo "success";

$notify_data = json_decode($notify_data, true);
if ($notify_data["result"] != "success")
{
  log::instance()->error("ret: 支付失败");
  exit;
}

$note = json_decode(base64_decode($notify_data["order_id_com"]), true);
$ret = recharge (
  $pf_info["PF"],
  $note["sid"],
  $note["odr"],
  $note["uid"],
  $note["item"],
  $notify_data["amount"],
  $notify_data["order_id"],
  0
);
log::instance()->debug("ret: " . $ret);
?>
