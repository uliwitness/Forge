#!/usr/bin/env Forge
on startUp
	read from file "/etc/apache2/httpd.conf"
	if the result is not empty then
		put the result
	else
		put it
	end if
	write "So you want to run HyperTalk scripts, eh?" to file "/Users/uli/forge_test_15.txt"
	if the result is not empty then
		put the result
	end if
end startUp
