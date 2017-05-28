<?hc
	set the filetype of the page to "html"

	on printPageTitle
		output htmlencoded(the title of the page)
	end printPageTitle
?><html>
<head>
<meta charset="UTF-8" />
<title><?hc printPageTitle
?></title>
</head>
<body>
