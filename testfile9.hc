on startUp
	put quoteIt("foo") into myVar
	put myVar
end startup

function quoteIt str
	return quote & str & quote
end quoteIt

on testHandler
	put "Trial of a" && quoteIt("Time Lord") & return
end testHandler