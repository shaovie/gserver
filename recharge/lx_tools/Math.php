<?php
class Math {
	private static $primes = array(2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97);
	public static function bin2int($string){
		$result = '0';
		$n = strlen($string);
		do {
			$result = bcadd(bcmul($result,'256'), ord($string{--$n}));
		} while ($n > 0);
		return $result;
	}
	public static function int2bin($num){
		$result = '';
		do {
			$result .= chr(bcmod($num, '256'));
			$num = bcdiv($num, '256');
		} while ($num != '0');
		return $result;
	}
	public static function bitLen($num) {
		$tmp = self::int2bin($num);
		$len = strlen($tmp) * 8;
		$tmp = ord($tmp {strlen($tmp) - 1});
		if (!$tmp)$len -= 8;
		else while(!($tmp & 0x80)){
				$len --;
				$tmp <<= 1;
			}
		return $len;
	}
	public static function byte2int($byte){
		if(is_array($byte)) $byte = implode('', $byte);
		$byte = ltrim($byte, '0');
		$len = strlen($byte);
		$sign = substr($byte, 0, 1) == '-' ? true : false;
		$arr = Array();
		
		$intlen = intval(($len + 3) / 4);
		for($i = 0, $k = $len - 1; $i < $intlen; $i++){
			$arr[$i] = ord($byte[$k--]) & 0xff;
			$n = min(3,  $k + 1);
			for ($j=8; $j <= 8 * $n; $j += 8)
				$arr[$i] |= ((ord($byte[$k--]) & 0xff) << $j);
			$arr[$i] = bcmul($arr[$i], bcpow('4294967296', $i));
		}
		$sum = '0';
		foreach($arr as $v) $sum = bcadd($sum, $v);
		if($sign == true && $sum != '0') $sum = '-' . $sum;
		return $sum;
	}
	public static function int2byte($num){
		$arr = array();
		$bit = '';
		while(bccomp($num, '0') > 0){
			$asc = bcmod($num, '256');
			$bit = chr($asc) . $bit;
			$num = bcdiv($num, '256');
		}
		return $bit;
	}
	public static function dec2hex($num){
		$char = '0123456789abcdef';
		while (bccomp($num, '0') > 0){
			$k = bcmod($num, '16');
			$hex = $char[intval($k)] . $hex;
			$num = bcdiv($num, '16');
		}
		return $hex;
	}
	public static function hex2dec($num){
		$char = '0123456789abcdef';
		$num = strtolower($num);
		$len = strlen($num);
		$sum = '0';
		for($i = $len - 1, $k = 0; $i >= 0; $i--, $k++){
			$index = strpos($char, $num[$i]);
			$sum = bcadd($sum, bcmul($index, bcpow('16', $k)));
		}
		return $sum;
	}
	public static function mr_test($num, $base) { //Miller-Rabin Test 
		if ($num == '1') return false;
		$tmp = bcsub($num, '1');
		
		$k = 0;
		while(bcmod($tmp, '2') == '0') {
			$k++;
			$tmp = bcdiv($tmp, '2');
		}
		$tmp = bcpowmod($base, $tmp, $num);
		if ($tmp == '1') return true;	
		
		while ($k--){
			if (bcadd($tmp, '1') == $num) return true;
			$tmp = bcpowmod($tmp, '2', $num);
		}
		return false;
	}
	public static function is_prime($num) {
		if(in_array($num, self::$primes)) return true;
		for ($i = 0; $i < 7; $i++)
			if(self::mr_test($num, self::$primes[$i]) == false) return false;
		return true;
	}
	public static function get_prime($bit_len) {
		$k = intval($bit_len / 8);
		$cnt = $bit_len % 8;
		do {
			$str = '';
			for ($i = 0; $i < $k; $i++) $str .= chr(mt_rand(0, 0xff));
			$n = mt_rand(0, 0xff) | 0x80;
			$n >>= 8 - $cnt;
			$str .= chr($n);
			$num = self::bin2int($str);
			
			if (bcmod($num, '2') == '0') $num = bcadd($num, '1');
		
			while (self::is_prime($num) == false) $num = bcadd($num, '2');
		} while (self::bitLen($num) != $bit_len);
		return $num;
	}
	
	public static function get_gcd($a, $b){ //Euclidean
		while($b != '0'){
			$k = $b;
			$b = bcmod($a, $b);
			$a = $k;
		}
		return $a;
	}

	public static function get_modinv($num, $m){ // 1/$num mod $m
		$x = '1';
		$y = '0';
		$k = $m;
		do {
			$tmp = bcmod($num, $k);
			$q = bcdiv($num, $k);
			$num = $k;
			$k = $tmp;
			
			$tmp = bcsub($x, bcmul($y, $q));
			$x = $y;
			$y = $tmp;
		} while ($k != '0');
		if (bccomp($x, '0') < 0) $x = bcadd($x, $m);
		return $x;
	}
}