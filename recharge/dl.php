<?php
include_once("log.php");
include_once("recharge.php");

function dl_recharge($AppId)
{
  log::init('./log', 'dl_log');
  $uri = $_SERVER['REQUEST_URI'];
  log::instance()->debug("new con: $uri");

  $config = include "config.php";

  $pf_info = $config["dl"][$AppId];
  $str = "order=".stripslashes($_GET["order"])
    ."&money=".stripslashes($_GET["money"])
    ."&mid=".stripslashes($_GET["mid"])
    ."&time=".stripslashes($_GET["time"])
    ."&result=".stripslashes($_GET["result"])
    ."&ext=".stripslashes($_GET["ext"])
    ."&key=".$pf_info["AppKey"];

  $sign = strtolower(md5($str));
  if ($sign != stripslashes($_GET["signature"]))
  {
    log::instance()->error("ret: 签名无效");
    exit;
  }
  echo "success";

  $note = json_decode(stripslashes($_GET["ext"]), true);
  $ret = recharge (
    $pf_info["PF"],
    $note["sid"],
    $note["odr"],
    $note["uid"],
    $note["item"],
    stripslashes($_GET["money"]),
    stripslashes($_GET["order"]),
    0
  );
  log::instance()->debug("ret: " . $ret);

  if ($ret != "SUCCESS" && $ret != "TRADE_NO NOT EXIST")
  {
    $str = "app_id=$AppId"
      ."&mid=".stripslashes($_GET["mid"])
      ."&order_no=".stripslashes($_GET["order"])
      ."&key=".$pf_info["AppKey"];
    $sign = strtolower(md5($str));

    $url = "http://connect.d.cn/open/pay-async/refund"
      ."?app_id=".urlencode($AppId)
      ."&mid=".urlencode(stripslashes($_GET["mid"]))
      ."&order_no=".urlencode(stripslashes($_GET["order"]))
      ."&sig=".urlencode($sign);

    $response = file_get_contents($url);
    log::instance()->debug("refund: order:".stripslashes($_GET["order"])." ret:$response");
  }
}
?>
