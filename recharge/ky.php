<?php
include_once("log.php");
include_once("recharge.php");
include_once("ssl.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'ky_log');
$uri = $_SERVER['REQUEST_URI'];
$body = file_get_contents('php://input');
log::instance()->debug("new con: $uri $body");

$AppID = "5183";
$pf_info = $config["ky"][$AppID];

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
  echo "failed";
  exit;
}
// end verify

parse_str(publickey_decodeing(base64_decode(stripslashes($_POST["notify_data"])), $pubkey), $notify_data);
if ($notify_data["dealseq"] != stripslashes($_POST["dealseq"]))
{
  log::instance()->error("ret: 数据不一致");
  echo "failed";
  exit;
}

if ($notify_data["payresult"] != 0)
{
  log::instance()->error("ret: 支付失败");
  echo "success";
  exit;
}

// ky 不支持json串, 长度限制64
list($trade_no, $sid, $uid, $item) = split(",", $notify_data["dealseq"]);
$ret = recharge (
  $pf_info["PF"],
  $sid,
  $trade_no,
  $uid,
  $item,
  $notify_data["fee"],
  stripslashes($_POST["orderid"]),
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
  echo "success";
else
  echo "failed";
?>
