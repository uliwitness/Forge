#!/usr/bin/env forge
on startUp
	handlerWithInvalidCommand "foo"
end startUp

on handlerWithInvalidEnd
	put "Even this handler should work, though."
	end handlerWithoutAProperEnd

on handlerWithInvalidCommand theParam
	put "This one is still fine..."
	argl bargl holly cargill
end handlerWithInvalidCommand
