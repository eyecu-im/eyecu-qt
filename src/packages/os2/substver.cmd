/* Version substitution script */

parse arg FILENAME VERSION

say 'filename="'||FILENAME||'"'
say 'version="'||VERSION||'"'


parse var VERSION MAJOR '.' MINOR '.' PATCH '.' REV

say "MAJOR="||MAJOR
say "MINOR="||MINOR
say "PATCH="||PATCH
say "REV="||REV

IS = FILENAME||".template"
OS = FILENAME||".wis"

say 'input file name: "'||IS||'"'
say 'output file name: "'||OS'"'

"del "||OS

if stream(IS,'c','open read') = 'READY:' then
do
	if stream(OS,'c','open write') = 'READY:' then
	do
		do while lines(IS)
			LINE = linein(IS)
			LINE = REPLACE(LINE, "%VERSION%", VERSION)
			LINE = REPLACE(LINE, "%MAJOR%", MAJOR)
			LINE = REPLACE(LINE, "%MINOR%", MINOR)
			LINE = REPLACE(LINE, "%PATCH%", PATCH)
			LINE = REPLACE(LINE, "%REV%", REV)
			rc = lineout(OS, LINE)
		end
		rc = stream(OS,'c','close')
	end
	else say "cannot open: "||OS||" for writing"
	rc = stream(IS,'c','close')
end
else SAY "cannot open: "||IS||" for reading"

return

/* Function to replace a substring with a string */
REPLACE:
	POSITION = pos(arg(2), arg(1))
	if POSITION>0 then
	do
		FIRST = left(arg(1), POSITION-1)
		LAST = right(arg(1), length(arg(1))-POSITION-length(arg(2))+1)
		return FIRST||arg(3)||LAST
	end
	return arg(1)
