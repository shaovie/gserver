<?php
include_once("log.php");
include_once("recharge.php");
include_once("ssl.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'hw_log');
$uri = $_SERVER['REQUEST_URI'];
$body = file_get_contents('php://input');
log::instance()->debug("new con: $uri $body");

$AppID = "10192320";
$pf_info = $config["hw"][$AppID];

// RSA verify
$params = $_POST;
ksort($params);
$str = "";
foreach ($params as $key => $value)
{
  if ($key == "sign") continue;
  if (strlen($str) == 0)
    $str = $key."=".stripslashes($value);
  else
    $str = $str."&".$key."=".stripslashes($value);
}
$sign = base64_decode(stripslashes($_POST["sign"]));

$pubkey = "-----BEGIN PUBLIC KEY-----\r\n".chunk_split($pf_info["PubKey"], 64, "\r\n")."-----END PUBLIC KEY-----";
if (!rsa_verify($str, $sign, $pubkey))
{
  log::instance()->error("ret: 签名无效");
  echo "{\"result\":1}";
  exit;
}
// end verify

if (stripslashes($_POST["result"]) != '0')
{
  log::instance()->error("ret: 支付失败");
  echo "{\"result\":0}";
  exit;
}

$note = json_decode(stripslashes($_POST["extReserved"]), true);
$ret = recharge (
  $pf_info["PF"],
  $note["sid"],
  stripslashes($_POST["requestId"]),
  $note["uid"],
  $note["item"],
  stripslashes($_POST["amount"]),
  stripslashes($_POST["orderId"]),
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
  echo "{\"result\":0}";
else if ($ret == "PARAM ERROR")
  echo "{\"result\":98}";
else
  echo "{\"result\":99}";
?>
