<?php
require 'Math.php';
class RSAUtil{
	public $keylen = 64;
	
	
	private $_key = array();
	
	/**
	 * 获取随机质数
	 * @return
	 */
	public function get_primes(){
		return Math::get_prime($this->keylen);
	}
	
	/**
	 * 通过P,Q计算N值
	 * @param 一个素数p
	 * @param 一个素数q
	 * @return 返回P*Q的值n
	 */
	public function get_n($p, $q){
		return bcmul($p, $q);
	}
	
	/**
	 * 通过P,Q计算ran值 modkey
	 * @param 一个素数p,不能为空
	 * @param 一个素数q,不能为空
	 * @return 返回(P-1)*(Q-1)的值ran
	 */
	public function get_ran($p, $q){
		return bcmul(bcsub($p, '1'), bcsub($q, '1'));
	}
	
	/**
	 * 获取公钥
	 * @param $ran
	 * @return ;
	 */
	public function get_public_key($ran){
		$e = '0';
		do{
			$tmp = Math::get_prime($this->keylen);
			if(Math::get_gcd($tmp, $ran) == '1') $e = $tmp;
		}while(Math::get_gcd($tmp, $ran) != '1');
		return $e;
	}
	
	/**
	 * 获取私钥
	 * @param $ran
	 * @param $public_key
	 * @return
	 */
	public function get_private_key($ran, $public_key) {
		return Math::get_modinv($public_key, $ran);
	}
	/**
	 * 加密方法
	 * @param $string 需要加密的明文字符
	 * @param $e 公钥
	 * @param $n
	 * @return String
	 */
	public function encrypt($string, $e, $n){
		$bitlen = $this->keylen * 2 - 1;
		$bitlen = intval($bitlen / 8);
		$len = strlen($string);
		$arr = array();
		for($i = 0; $i < $len; $i += $bitlen)
			$arr[] = substr($string, $i, $bitlen);
		$index = count($arr) - 1;
		$len = strlen($arr[$index]);
		if($len < $bitlen) $arr[$index] = $arr[$index] . str_repeat(' ', $bitlen - $len);
		$data = array();
		foreach ($arr as $v){
			$v = Math::byte2int($v);
			$v = bcpowmod($v, $e, $n);
			$data[] = Math::dec2hex($v);
		}
		
		return implode(' ', $data);
	}
	/**
	 * 解密方法
	 * @param $string 需要解密的密文字符
	 * @param $d
	 * @param $n
	 * @return String
	 */
	public function decrypt($string, $d, $n){
		//解决某些机器验签时好时坏的bug
		//BCMath 里面的函数 有的机器php.ini设置不起作用
		//要在RSAUtil的方法decrypt 加bcscale(0);这样一行代码才行
		//要不有的机器计算的时候会有小数点 就会失败
		bcscale(0);
		
		$bln = $this->keylen * 2 - 1;
		$bitlen = ceil($bln / 8);
		$arr = explode(' ', $string);
		$data = '';
		foreach($arr as $v){
			$v = Math::hex2dec($v);
			$v = bcpowmod($v, $d, $n);
			$data .= Math::int2byte($v);
		}
		return trim($data);
	}
}
?>