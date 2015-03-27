<?php
function publickey_decodeing($crypttext, $publickey)
{
  $pubkeyid = openssl_get_publickey($publickey);
  $ret = openssl_public_decrypt($crypttext, $sourcestr, $pubkeyid, OPENSSL_PKCS1_PADDING);
  openssl_free_key($pubkeyid);

  if ($ret)
    return $sourcestr;

  return FALSE;
}

function rsa_verify($sourcestr, $signstr, $publickey)
{
  $pubkeyid = openssl_get_publickey($publickey);
  $verify = openssl_verify($sourcestr, $signstr, $pubkeyid);
  openssl_free_key($pubkeyid);
  return $verify;
}

function publickey_decodeing_sectionalized($crypttext, $interval, $publickey)
{
  $pubkeyid = openssl_get_publickey($publickey);
  $output = "";
  while ($crypttext) {
    $input = substr($crypttext, 0, $interval);
    $crypttext = substr($crypttext, $interval);
    openssl_public_decrypt($input, $out, $pubkeyid);
    $output .= $out;
  }
  openssl_free_key($pubkeyid);
  return $output;
}
?>
