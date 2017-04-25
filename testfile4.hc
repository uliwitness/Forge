#!/usr/bin/env Forge
on testx str1,str2,str3
  put "1,2,3,4" into str1
  put "" into str2
  put  number of items of str1 into icnt -- 4
  -- comment out the following 3 lines once we can insert
  -- into nonexistent lines:
  --repeat  icnt-1                  --don't preset with trailing comma
  --  put "," after str2            --append delimiter
  --end repeat
  put str2 into str3 -- str2 == str3 == ",,,"
  repeat with i from 1 to icnt
    put item i of str1 into item  i  of str2
  end repeat
  put str2 &return& str3
end testx
