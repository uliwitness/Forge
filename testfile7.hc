on testx
	put quoteIt("foo") into myVar
	return myVar
end testx

function quoteIt str
	return quote & str & quote
end quoteIt