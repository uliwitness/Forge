#!/usr/bin/env forge
on startUp
	put "a": "b" into myVar
	put "foo" into entry "bar" of myVar
	output myVar & newline &newline
	put "1": "first" into yourVar
	put "second" into entry (number of entries of yourVar +1) of yourVar
	output yourVar & newline &newline
	put "../foo/bar/baz" into thePath
	delete character 1 to 3 of thePath
	output thePath & newline
end startUp
