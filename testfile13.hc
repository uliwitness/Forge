#!/usr/bin/env Forge
on startup
	global gCounter
	if gCounter is empty then put 1 into gCounter
	put “Bored…” && gCounter
	add 1 to gCounter
end startup
