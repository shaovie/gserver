<?php
include_once("log.php");
include_once("recharge.php");
include_once("ssl.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'wdj_log');
$uri = $_SERVER['REQUEST_URI'];
$body = file_get_contents('php://input');
log::instance()->debug("new con: $uri $body");

$AppID = "100013257";
$pf_info = $config["wdj"][$AppID];

// RSA verify
$content = stripslashes($_POST["content"]);
$sign = base64_decode(stripslashes($_POST["sign"]));
$pem = chunk_split($pf_info["PubKey"], 64, "\n");
$pubkey = "-----BEGIN PUBLIC KEY-----\n".$pem."-----END PUBLIC KEY-----";
if (!rsa_verify($content, $sign, $pubkey))
{
  log::instance()->error("ret: 签名无效");
  echo "fail";
  exit;
}
// end verify

$content = json_decode($content, true);
$note = json_decode($content["out_trade_no"], true);
$ret = recharge (
  $pf_info["PF"],
  $note["sid"],
  $note["odr"],
  $note["uid"],
  $note["item"],
  $content["money"] / 100,
  $content["orderId"],
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
  echo "success";
else
  echo "fail";
?>
