#!/usr/bin/env Forge
on startUp
	put empty into myVar
	repeat with x = 1 to 10
		put "meh," after myVar
	end repeat
	put myVar
end startUp
