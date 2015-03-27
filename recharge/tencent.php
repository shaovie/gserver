<?php
include_once("log.php");
include_once("recharge.php");
include_once("tx_tools/SnsSigCheck.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'tencent_log');
$uri = $_SERVER['REQUEST_URI'];
$method = strtoupper($_SERVER['REQUEST_METHOD']);
if ("GET" == $method)
  $params = $_GET;
else if ("POST" == $method)
{
  $body = file_get_contents('php://input');
  $params = $_POST;
}
log::instance()->debug("new con: $uri $body");

$pf_info = $config["tencent"][stripslashes($params["appid"])];
$api = substr($uri, 0, strcspn($uri, '?'));
if (!SnsSigCheck::verifySig($method, $api, $params, $pf_info["AppKey"].'&', $params["sig"]))
{
  log::instance()->error("ret: 签名无效");
  $obj->ret = 4;
  $obj->msg = "签名无效";
  echo json_encode($obj);
  exit;
}

$just_record = true;
if ($just_record)
  $ret = "SUCCESS";
else
{
  list($note, $t1, $t2) = split("\*", stripslashes($params["appmeta"]));
  $note = json_decode(stripslashes($note), true);
  $ret = recharge (
    $pf_info["PF"],
    stripslashes($params["zoneid"]),
    $note["odr"],
    $note["uid"],
    $note["item"],
    $params["amt"] / 100 + $params["payamt_coins"] / 10 + $params["pubacct_payamt_coins"] / 10,
    stripslashes($params["billno"]),
    0
  );
}
log::instance()->debug("ret: " . $ret);

$obj->ret = 4;
$obj->msg = $ret;
if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
{
  $obj->ret = 0;
  $obj->msg = 'OK';
}
echo json_encode($obj);
?>
