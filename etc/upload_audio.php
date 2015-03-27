<?php
if ($_FILES["file"]["error"] > 0)
{
  echo "Return Code: " . $_FILES["file"]["error"] . "<br />";
}if ($_FILES["file"]["size"] / 1024 > 1024)
{
  echo $_FILES["file"]["name"] . " too large. ";
}else
{
  if (file_exists("audio/" . $_FILES["file"]["name"]))
  {
    echo $_FILES["file"]["name"] . " already exists. ";
  }
  else
  {
    move_uploaded_file($_FILES["file"]["tmp_name"], "audio/" . $_FILES["file"]["name"]);
    #echo "Stored in: " . "audio/" . $_FILES["file"]["name"];
  }
}
?>
