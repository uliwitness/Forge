#!/usr/bin/env forge
on startUp
	put 1: "first" 2: "second" 3: "third" into theList
	put 15 gigabytes into theMemory
	output "Length of string:" && the number of characters of theList & newline
	output "Number of entries:" && the number of entries of theList & newline
	repeat for each entry currEntry in theList
		output currEntry & newline
	end repeat
end startUp
