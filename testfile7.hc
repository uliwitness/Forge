#!/usr/bin/env Forge
on startup
	put quoteIt("foo") into myVar
	put myVar
end startup

function quoteIt str
	return quote & str & quote
end quoteIt
