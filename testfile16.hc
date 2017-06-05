<?hc
	set the title of the page to "This is the <title>"
	set the slug of the page to "this-is-the-title"
	--set the filetype of the page to "rss"
	set the filename of the page to "xmls/gobblegobble.xml"
	copy file "testfile16.hc" to "testfile16 copy.hc"

	use "testfile16_header.hc"
?>
<?hc repeat with x = 1 to 5 ?>
<b>This is page content.</b><br /><?hc
end repeat ?>
<?hc repeat with x = 1 to 5
put "<i>This is page content.</i><br />" &newline
end repeat ?>
<?hc use "testfile16_footer.hc" ?>
