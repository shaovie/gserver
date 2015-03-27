<?php
include_once("log.php");
include_once("recharge.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'uc_log');
$uri = $_SERVER['REQUEST_URI'];
$body = file_get_contents('php://input');
log::instance()->debug("new con: $uri $body");

$body = json_decode(urldecode($body), true);
if (!isset($body["data"]))
{
  log::instance()->error("ret: 无效请求");
  echo "FAILURE";
  exit;
}
$pf_info = $config["uc"][$body["data"]["gameId"]];

ksort($body["data"]);
$str = $pf_info["cpId"];
foreach ($body["data"] as $key => $value)
  $str = "$str$key=$value";
$str = $str.$pf_info["AppKey"];

$sign = strtolower(md5($str));
if ($body["sign"] != $sign)
{
  log::instance()->error("ret: 签名无效");
  echo "FAILURE";
  exit;
}

echo "SUCCESS";

if ($body["data"]["orderStatus"] == 'F')
{
  log::instance()->error("ret: 支付失败");
  exit;
}

$note = json_decode($body["data"]["callbackInfo"], true);
$ret = recharge (
  $pf_info["PF"],
  $note["sid"],
  $note["odr"],
  $note["uid"],
  $note["item"],
  $body["data"]["amount"],
  $body["data"]["orderId"],
  0
);
log::instance()->debug("ret: " . $ret);
?>
