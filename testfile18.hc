#!/usr/bin/env forge
on startUp
	start recording output into myOutput
	output the parameters
	stop recording output into myOutput
	output "Stuff printed: «" && myOutput && "»" & newline
end startUp
