#!/usr/bin/env forge
on startUp
	put "a": "b" into myVar
	put "foo" into entry "bar" of myVar
	output myVar & newline
end startUp
