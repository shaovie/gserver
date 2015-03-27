<?php
/**
 * PHP SDK for  OpenAPI V3
 *
 * @version 3.0.9
 * @author open.qq.com
 * @copyright © 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *				 3.0.9 | coolinchen| 2013-05-30 11:14:12 | remove that SDK check whether the openkey is empty
 *				 3.0.8 | coolinchen| 2013-05-03 15:30:12 | resolve a bug: the signature verification is not passed when the method is GET
 *				 3.0.7 | coolinchen| 2013-02-28 11:50:20 | modify response code 
 *				 3.0.6 | coolinchen| 2012-12-05 17:10:11 | support sending a request by Post
 *				 3.0.5 | coolinchen| 2012-10-08 11:20:12 | support printing request string and result 
 *				 3.0.4 | coolinchen| 2012-09-07 10:20:12 | support POST request in  "multipart/form-data" format
 *               3.0.3 | nemozhang | 2012-08-28 16:40:20 | support cpay callback sig verifictaion
 *               3.0.2 | sparkeli  | 2012-03-06 17:58:20 | add statistic fuction which can report API's access time and number to background server
 *               3.0.1 | nemozhang | 2012-02-14 17:58:20 | resolve a bug: at line 108, change  'post' to  $method
 *               3.0.0 | nemozhang | 2011-12-12 11:11:11 | initialization
 */

require_once 'SnsNetwork.php';
require_once 'SnsSigCheck.php';

/**
 * 如果您的 PHP 没有安装 cURL 扩展，请先安装 
 */
if (!function_exists('curl_init'))
{
  throw new Exception('OpenAPI needs the cURL PHP extension.');
}

/**
 * 如果您的 PHP 不支持JSON，请升级到 PHP 5.2.x 以上版本
 */
if (!function_exists('json_decode'))
{
  throw new Exception('OpenAPI needs the JSON PHP extension.');
}

/**
 * 错误码定义
 */
define('OPENAPI_ERROR_REQUIRED_PARAMETER_EMPTY', 1801); // 参数为空
define('OPENAPI_ERROR_REQUIRED_PARAMETER_INVALID', 1802); // 参数格式错误
define('OPENAPI_ERROR_RESPONSE_DATA_INVALID', 1803); // 返回包格式错误
define('OPENAPI_ERROR_CURL', 1900); // 网络错误, 偏移量1900, 详见 http://curl.haxx.se/libcurl/c/libcurl-errors.html

/**
 * 提供访问腾讯开放平台 OpenApiV3 的接口
 */
class OpenApiV3
{
  private $appid  = 0;
  private $appkey = '';
  private $server_name = '';
  private $format = 'json';
  private $is_stat = true;

  /**
   * 构造函数
   *
   * @param int $appid 应用的ID
   * @param string $appkey 应用的密钥
   */
  function __construct($appid, $appkey)
  {
    $this->appid = $appid;
    $this->appkey = $appkey;
  }

  public function setServerName($server_name)
  {
    $this->server_name = $server_name;
  }

  public function setIsStat($is_stat)
  {
    $this->is_stat = $is_stat;
  }

  /**
   * 执行API调用，返回结果数组
   *
   * @param string $script_name 调用的API方法，比如/v3/user/get_info，参考 http://wiki.open.qq.com/wiki/API_V3.0%E6%96%87%E6%A1%A3
   * @param array $params 调用API时带的参数
   * @param string $method 请求方法 post / get
   * @param string $protocol 协议类型 http / https
   * @return array 结果数组
   */
  public function api($script_name, $params, $cookie, $method='post', $protocol='http')
  {
    // 检查 openid 是否为空
    if (!isset($params['openid']) || empty($params['openid']))
    {
      return array(
        'ret' => OPENAPI_ERROR_REQUIRED_PARAMETER_EMPTY,
        'msg' => 'openid is empty');
    }
    // 检查 openid 是否合法
    if (!self::isOpenId($params['openid']))
    {
      return array(
        'ret' => OPENAPI_ERROR_REQUIRED_PARAMETER_INVALID,
        'msg' => 'openid is invalid');
    }

    // 无需传sig, 会自动生成
    unset($params['sig']);

    // 添加一些参数
    $params['appid'] = $this->appid;
    $params['format'] = $this->format;

    // 生成签名
    $secret = $this->appkey . '&';
    $sig = SnsSigCheck::makeSig( $method, $script_name, $params, $secret);
    $params['sig'] = $sig;

    $url = $protocol . '://' . $this->server_name . $script_name;

    //通过调用以下方法，可以打印出最终发送到openapi服务器的请求参数以及url，默认为注释
    //self::printRequest($url,$params,$method);


    // 发起请求
    $ret = SnsNetwork::makeRequest($url, $params, $cookie, $method, $protocol);

    if (false === $ret['result'])
    {
      $result_array = array(
        'ret' => OPENAPI_ERROR_CURL + $ret['errno'],
        'msg' => $ret['msg'],
      );
    }

    $result_array = json_decode($ret['msg'], true);

    // 远程返回的不是 json 格式, 说明返回包有问题
    if (is_null($result_array)) {
      $result_array = array(
        'ret' => OPENAPI_ERROR_RESPONSE_DATA_INVALID,
        'msg' => $ret['msg']
      );
    }

    // 统计上报
    if ($this->is_stat)
    {
      $stat_params = array(
        'appid' => $this->appid,
        'pf' => $params['pf'],
        'rc' => $result_array['ret'],
        'svr_name' => $this->server_name,
        'interface' => $script_name,
        'protocol' => $protocol,
        'method' => $method,
      );
    }

    //通过调用以下方法，可以打印出调用openapi请求的返回码以及错误信息，默认注释
    //self::printRespond($result_array);

    return $result_array;
  }

  /**
   * 执行上传文件API调用，返回结果数组
   *
   * @param string $script_name 调用的API方法，比如/v3/user/get_info， 参考 http://wiki.open.qq.com/wiki/API_V3.0%E6%96%87%E6%A1%A3
   * @param array $params 调用API时带的参数，必须是array
   * @param array $array_files 调用API时带的文件，必须是array，key为openapi接口的参数，value为"@"加上文件全路径的字符串
   *															  举例 array('pic'=>'@/home/xxx/hello.jpg',...);
   * @param string $protocol 协议类型 http / https
   * @return array 结果数组
   */
  public function apiUploadFile($script_name, $params, $array_files, $protocol='http')
  {
    // 检查 openid 是否为空
    if (!isset($params['openid']) || empty($params['openid']))
    {
      return array(
        'ret' => OPENAPI_ERROR_REQUIRED_PARAMETER_EMPTY,
        'msg' => 'openid is empty');
    }
    // 检查 openid 是否合法
    if (!self::isOpenId($params['openid']))
    {
      return array(
        'ret' => OPENAPI_ERROR_REQUIRED_PARAMETER_INVALID,
        'msg' => 'openid is invalid');
    }

    // 无需传sig, 会自动生成
    unset($params['sig']);

    // 添加一些参数
    $params['appid'] = $this->appid;
    $params['format'] = $this->format;

    // 生成签名
    $secret = $this->appkey . '&';
    $sig = SnsSigCheck::makeSig( 'post', $script_name, $params, $secret);
    $params['sig'] = $sig;

    //上传文件,图片参数不能参与签名
    foreach($array_files as $k => $v)
    {
      $params[$k] = $v ;
    }

    $url = $protocol . '://' . $this->server_name . $script_name;
    $cookie = array();

    //通过调用以下方法，可以打印出最终发送到openapi服务器的请求参数以及url，默认注释
    //self::printRequest($url, $params,'post');

    // 发起请求
    $ret = SnsNetwork::makeRequestWithFile($url, $params, $cookie, $protocol);

    if (false === $ret['result'])
    {
      $result_array = array(
        'ret' => OPENAPI_ERROR_CURL + $ret['errno'],
        'msg' => $ret['msg'],
      );
    }

    $result_array = json_decode($ret['msg'], true);

    // 远程返回的不是 json 格式, 说明返回包有问题
    if (is_null($result_array)) {
      $result_array = array(
        'ret' => OPENAPI_ERROR_RESPONSE_DATA_INVALID,
        'msg' => $ret['msg']
      );
    }

    // 统计上报
    if ($this->is_stat)
    {
      $stat_params = array(
        'appid' => $this->appid,
        'pf' => $params['pf'],
        'rc' => $result_array['ret'],
        'svr_name' => $this->server_name,
        'interface' => $script_name,
        'protocol' => $protocol,
        'method' => 'post',
      );
    }

    //通过调用以下方法，可以打印出调用openapi请求的返回码以及错误信息,默认注释
    //self::printRespond($result_array);

    return $result_array;
  }

  /**
   * 打印出请求串的内容，当API中的这个函数的注释放开将会被调用。
   *
   * @param string $url 请求串内容
   * @param array $params 请求串的参数，必须是array
   * @param string $method 请求的方法 get / post
   */
  private function printRequest($url, $params,$method)
  {
    $query_string = SnsNetwork::makeQueryString($params);
    if($method == 'get')
    {
      $url = $url."?".$query_string;
    }
    echo "\n============= request info ================\n\n";
    print_r("method : ".$method."\n");
    print_r("url    : ".$url."\n");
    if($method == 'post')
    {
      print_r("query_string : ".$query_string."\n");
    }
    echo "\n";
    print_r("params : ".print_r($params, true)."\n");
    echo "\n";
  }

  /**
   * 打印出返回结果的内容，当API中的这个函数的注释放开将会被调用。
   *
   * @param array $array 待打印的array
   */
  private function printRespond($array)
  {
    echo "\n============= respond info ================\n\n";
    print_r($array);
    echo "\n";
  }

  /**
   * 检查 openid 的格式
   *
   * @param string $openid openid
   * @return bool (true|false)
   */
  private static function isOpenId($openid)
  {
    return (0 == preg_match('/^[0-9a-fA-F]{32}$/', $openid)) ? false : true;
  }





}

// end of script
