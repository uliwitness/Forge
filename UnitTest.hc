on startup
	put "Running tests..." &newline

	put "this,that,more" into theItems
	delete item 1 of theItems
	if theItems is not "that,more" then
		put "*** BUILD FAILED ***" &newline
		return ""
	end if

	put "this,that,more" into theItems
	delete item 2 of theItems
	if theItems is not "this,more" then
		put "*** BUILD FAILED ***" &newline
		return ""
	end if

	put "this,that,more" into theItems
	delete item 3 of theItems
	if theItems is not "this,that" then
		put "*** BUILD FAILED ***" &newline
		return ""
	end if

	put "this,that,more" into theItems
	delete item 1 through 2 of theItems
	if theItems is not "more" then
		put "*** BUILD FAILED ***" &newline
		return ""
	end if

	put "this,that,more" into theItems
	delete item 2 through 3 of theItems
	if theItems is not "this" then
		put "*** BUILD FAILED ***" &newline
		return ""
	end if

	put "this,that,more" into theItems
	delete item 1 through 3 of theItems
	if theItems is not empty then
		put "*** BUILD FAILED ***" &newline
		return ""
	end if

	put "Tests all ran successfully." &newline
end startup