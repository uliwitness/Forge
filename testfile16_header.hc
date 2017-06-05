<?hc
	set the filetype of the page to "html"

	set the resources of the page to "1": ("filename": "../theme/OJLF_Orange-Header2.png" "destination": "OJLF_Orange-Header2.png") "2": ("filename": "../theme/quote.png" "destination": "quote.png") "3": ("filename": "../theme/rays.png" "destination": "rays.png") "4": ("filename": "../theme/stitches.png" "destination": "stitches.png") "5": ("filename": "../theme/diagonal-stripes.png" "destination": "diagonal-stripes.png") "6": ("filename": "../theme/halftone-dark-small.png" "destination": "halftone-dark-small.png") "7": ("filename": "../theme/style.css" "destination": "style.css")

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
