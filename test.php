<?php
$str = "aaazz.bbbzz.ccczz.";
$pattern = "/(([a-z]+)([a-z]+)\\.)+/";
$ret = preg_match($pattern,$str,$matches);
var_dump($ret);
print_r($matches);
?>