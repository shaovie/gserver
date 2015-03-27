<?
date_default_timezone_set('PRC'); //东八时区
class log 
{
  private static $log_fp;
  private static $log_dir;
  private static $log_file;

  private static $log;

  private function __construct() 
  {
    if (!is_dir(self::$log_dir))
      mkdir(self::$log_dir, 0777, true);
    self::$log_fp = fopen(self::$log_dir."/".self::$log_file, "a+");
  }
  function __destruct() 
  {
    @fclose(self::$log_fp);
  }

  private function write_to_file($level, $str)
  {
    $format_str = '[' . date('Y-m-d H:i:s') . "]:[" . getmypid() . "]:[" . $level . "]:" . $str . "\n";
    @fwrite(self::$log_fp, $format_str, strlen($format_str));
  }

  public function debug($str) 
  {
    $this->write_to_file("DEBUG", $str);
  }

  public function error($str) 
  {
    $this->write_to_file("ERROR", $str);
  }

  public static function init($dir, $file)
  {
    self::$log_dir = $dir;
    self::$log_file = $file;
  }
  public static function instance()
  {
    if(!isset(self::$log))
    {
      self::$log = new log();
    }
    return self::$log;
  }
}
?>
