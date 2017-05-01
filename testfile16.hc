<html>
<head>
<title>This is a web page</title>
</head>
<body>
<?hc repeat with x = 1 to 5 ?>
<b>This is page content.</b><br /><?hc
end repeat ?>
<?hc repeat with x = 1 to 5
put "<i>This is page content.</i><br />" &newline
end repeat ?>
</body>
</html>
