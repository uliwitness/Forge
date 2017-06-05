#!/usr/bin/env forge
on startUp
	put 1: "first" 2: "second" 3: "third" into theList
	output "Number of entries:" && number of entries of theList & newline
	repeat for each entry currEntry in theList
		output currEntry & newline
	end repeat
end startUp
