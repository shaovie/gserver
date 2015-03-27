<?php
include_once("log.php");
include_once("recharge.php");
include_once("tx_tools/OpenApiV3.php");

$config = include "config.php";

header("Content-type: text/html; charset=utf-8");
log::init('./log', 'qq_log');
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
$appid = '1102506911';
$pf_info = $config["tencent"][$appid];

$openApi = new OpenApiV3($appid, $pf_info['AppKey']);
$openApi->setServerName('openapi.tencentyun.com');
// 查询余额
$pay_params = array (
  'openid' => $params['openid'],
  'openkey' => $params['openkey'],
  'pay_token' => $params['pay_token'],
  'ts' => time(),
  'pf' => $params['pf'],
  'zoneid' => $params['zoneid'],
  'pfkey' => $params['pfkey'],
);
$pay_cookie = array (
  'session_id' => 'openid',
  'session_type' => 'kp_actoken',
  'org_loc' => '/mpay/get_balance_m'
);
$result = $openApi->api($pay_cookie['org_loc'], $pay_params, $pay_cookie, 'GET', 'https');
$result['msg'] = urlencode($result['msg']);
$response = array ('type'=>1, 'ret'=>$result['ret']);
if ($result['ret'] !== 0)
{
  log::instance()->error("ret: 查询失败 " . urldecode(json_encode($result)));
  echo json_encode($response);
  exit;
} else if ($result['balance'] - $result['gen_balance'] < $params['save_num'])
{
  log::instance()->error("ret: 余额不足 " . urldecode(json_encode($result)));
  $response['msg'] = "余额不足";
  echo json_encode($response);
  exit;
}

// 扣除游戏币
$pay_params = array (
  'openid' => $params['openid'],
  'openkey' => $params['openkey'],
  'pay_token' => $params['pay_token'],
  'ts' => time(),
  'pf' => $params['pf'],
  'zoneid' => $params['zoneid'],
  'amt' => $params['save_num'],
  'pfkey' => $params['pfkey'],
);
$pay_cookie = array (
  'session_id' => 'openid',
  'session_type' => 'kp_actoken',
  'org_loc' => '/mpay/pay_m'
);
$result = $openApi->api($pay_cookie['org_loc'], $pay_params, $pay_cookie, 'GET', 'https');
$result['msg'] = urlencode($result['msg']);
$response = array ('type'=>1, 'ret'=>$result['ret']);
if ($result['ret'] === 0)
{
  log::instance()->debug("ret: 扣款成功 " . urldecode(json_encode($result)));
  $response['balance'] = $result['balance'];
} else
{
  log::instance()->error("ret: 扣款失败 " . urldecode(json_encode($result)));
  $response['err_code'] = $result['err_code'];
  echo json_encode($response);
  exit;
}

// 发钻石
$note = json_decode(stripslashes($params['appmeta']), true);
$ret = recharge (
  $pf_info["PF"],
  stripslashes($params["zoneid"]),
  $note["odr"],
  $note["uid"],
  $note["item"],
  $params["save_num"] / 10,
  $result["billno"],
  0
);
log::instance()->debug("ret: " . $ret);

if ($ret != "SUCCESS" && $ret != "TRADE_NO NOT EXIST")
{
  // 退款
  $pay_params = array (
    'openid' => $params['openid'],
    'openkey' => $params['openkey'],
    'pay_token' => $params['pay_token'],
    'ts' => time(),
    'pf' => $params['pf'],
    'zoneid' => $params['zoneid'],
    'amt' => $params['save_num'],
    'pfkey' => $params['pfkey'],
    'billno' => $result['billno']
  );
  $pay_cookie = array (
    'session_id' => 'openid',
    'session_type' => 'kp_actoken',
    'org_loc' => '/mpay/cancel_pay_m'
  );
  $result = $openApi->api($pay_cookie['org_loc'], $pay_params, $pay_cookie, 'GET', 'https');
  $result['msg'] = urlencode($result['msg']);
  $response = array ('type'=>2, 'ret'=>$result['ret']);
  if ($result['ret'] === 0)
    log::instance()->debug("ret: 退款成功 " . urldecode(json_encode($result)));
  else
    log::instance()->error("ret: 退款失败 " . urldecode(json_encode($result)));
}
echo json_encode($response);
?>
