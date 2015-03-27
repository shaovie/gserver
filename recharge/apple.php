<?php
include_once("log.php");
include_once("recharge.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'apple_log');
$uri = $_SERVER['REQUEST_URI'];
$body = file_get_contents('php://input');
log::instance()->debug("new con: $uri $body");

$body = json_decode(urldecode($body), true);

//$url = "https://buy.itunes.apple.com/verifyReceipt";
$url = "https://sandbox.itunes.apple.com/verifyReceipt";
$receipt = json_encode(array("receipt-data" => $body["receipt-data"]));
$response = sendPostData($url, $receipt);
$content = json_decode($response['content'], true);
if ($content["status"] !== 0)
{
  log::instance()->error("ret: 验证失败 errCode:".$content["status"]);
  echo "fail";
  exit;
}

$pf_info = $config["apple"]["appstore"];
$note = $content["receipt"];
$ret = recharge (
  $pf_info["PF"],
  $body["sid"],
  $body["odr"],
  $body["uid"],
  $note["product_id"],
  "apple",
  $note["transaction_id"],
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
  echo "success";
else
  echo "fail";

function sendPostData($url, $data)
{
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_POST, true);
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
  curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 180);
  curl_setopt($ch, CURLOPT_TIMEOUT, 600);
  $handles = curl_exec($ch);
  $header = curl_getinfo($ch);
  $header['content'] = $handles;
  curl_close($ch);
  return $header;
}
?>
