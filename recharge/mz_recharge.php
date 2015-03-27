<?php
include_once("log.php");
include_once("recharge.php");

function mz_recharge($AppId)
{
  log::init('./log', 'mz_log');
  $uri = $_SERVER['REQUEST_URI'];
  $body = file_get_contents('php://input');
  log::instance()->debug("new con: $uri $body");

  $config = include "config.php";

  $pf_info = $config["mz"][$AppId];
  $str = stripslashes($_POST["username"])
    ."|".stripslashes($_POST["change_id"])
    ."|".stripslashes($_POST["money"])
    ."|".$pf_info["AppKey"];

  $sign = stripslashes($_POST["hash"]);
  if (strtolower(md5($str)) != $sign)
  {
    log::instance()->error("ret: 签名无效");
    echo "0";
    exit;
  }

  $note = json_decode(stripslashes($_POST["object"]), true);
  $ret = recharge (
    $pf_info["PF"],
    $note["sid"],
    $note["odr"],
    $note["uid"],
    $note["item"],
    stripslashes($_POST["money"]),
    stripslashes($_POST["change_id"]),
    0
  );
  log::instance()->debug("ret: " . $ret);

  if ($ret == "SUCCESS" || $ret == "TRADE_NO NOT EXIST")
    echo "1";
  else
    echo "0";
}
?>
