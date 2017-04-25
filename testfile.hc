#!/usr/bin/env Forge
--------------------------------------------------------------------------------
-- testfile.hc
--
-- This is a test file demonstrating what code you can write in HyperC. You can
-- "compile" this into C code which then gets translated into a real honest-to
-- goodness executable.
--------------------------------------------------------------------------------

on testMe paramOne, paramTwo
	doSomething "this", "Jefferson AIRPLANE!" , 私の関数(12.5 + 3) -- test of function calls.
	
	put the long date into theDate
	put "|" && the abbr date after theDate
	put "|" && the short date after theDate
	--put "|" && the date after theDate
	put theDate
	
	-- now output two things to the console:
	-- put " Result: " & the result & return
	
	put char 1 to 2 of "ABCD"
	put " - "
	put char 2 to 3 of "ABCD"
	put " - "
	put char 3 to 4 of "ABCD"
	put return
	
	put number of words of "This is niftier than thou" &return
	put "Kill,me,now!" into theItems
	put "evil" into item 2 of theItems
	put theItems & return
	delete item 2 of theItems
	put theItems & return
	put item (number of items of theItems) of theItems
	
	-- get SysBeep(10)
	beep
	put quote & it & quote & linefeed
	
	repeat for each word theWord of "Hey  bore  " &tab& "  someone" &return& "else?"
		put theWord
	end repeat
	
	-- put [[ABAddressBook sharedAddressBook] me] into theMeObj
	-- return "You are: " & [[theMeObj valueForProperty: "First"] description] && [[theMeObj valueForProperty: "Last"] description] &return& "Param 1: " & paramOne
	
	return 1 + 2 * 3
end testMe

function 私の関数 what
	put "Num: " & what into foo
	return foo
end 私の関数

on doSomething a, b, c
	--put [[NSFileManager defaultManager] fileExistsAtPath: "/Users/witness/"] into cond
	--if cond = true then
	--	put "C is: " & quote & c & quote into c
	--else
	--	put "cond is: " & cond into c
	--end if
	put "" into counter
	repeat 10000000 times
		put "Uli" after counter
	end repeat
	--do "doSomethingFinishing"
	--put counter & return
	return c
end doSomething
