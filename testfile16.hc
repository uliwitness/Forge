<?hc global gAW_Title = "Test Web Page"

use "testfile16_header.hc" ?>
<?hc repeat with x = 1 to 5 ?>
<b>This is page content.</b><br /><?hc
end repeat ?>
<?hc repeat with x = 1 to 5
put "<i>This is page content.</i><br />" &newline
end repeat ?>
<?hc use "testfile16_footer.hc" ?>
