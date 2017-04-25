#!/usr/bin/env Forge
on startUp
	put 10 into theNumber
	repeat while theNumber > 0
		put theNumber
		put theNumber -1 into theNumber
	end repeat
	repeat for 10 times
		put "Looping..."
	end repeat
	repeat with x is 1 through 10
		put "Uppy" && x
	end repeat
	repeat with x is 10 down to 1
		put "Downy" && x
	end repeat
	repeat for each item theItem in "foo,bar,baz"
		put theItem
	end repeat
end startUp
