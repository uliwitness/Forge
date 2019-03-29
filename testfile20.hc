#!/usr/bin/env forge
-- this is the first handler. Cool eh?
-- you can add loads of comments to it.
on startUp
	-- this is a put command that creates an array:
	put "a": "b" into myVar
	put "foo" into entry "bar" of myVar
	output myVar & newline &newline
	put "1": "first" into yourVar
	put "second" into entry (number of entries of yourVar +1) of yourVar
	(* multi-line comments do not get glued together like single-liners,
	but ... *)
	(* multi-line comments can be attached to whatever
	is the next token: *)
	output yourVar & newline &newline
	put "../foo/bar/baz" into thePath
	delete character 1 to 3 of thePath
	output thePath & newline
end startUp
