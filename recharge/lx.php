<?php
include_once("log.php");
include_once("recharge.php");
include_once("lx_tools/IappDecrypt.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'lx_log');
$uri = $_SERVER['REQUEST_URI'];
$body = file_get_contents('php://input');
log::instance()->debug("new con: $uri $body");

$transdata = stripslashes($_POST["transdata"]);
$sign = stripslashes($_POST["sign"]);

$data = json_decode($transdata, true);
$pf_info = $config["lx"][$data["appid"]];

$tools = new IappDecrypt();
if ($tools->validsign($transdata, $sign, $pf_info["AppKey"]) != 0)
{
  log::instance()->error("ret: 签名无效");
  echo "FAILED";
  exit;
}

if ($data["result"] != 0)
{
  log::instance()->error("ret: 支付失败");
  echo "SUCCESS";
  exit;
}

list($trade_no, $sid, $uid, $item) = split(",", $data["cpprivate"]);
$ret = recharge (
  $pf_info["PF"],
  $sid,
  $data["exorderno"],
  $uid,
  $item,
  $data["money"] / 100,
  $data["transid"],
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
  echo "SUCCESS";
else
  echo "FAILED";
?>
