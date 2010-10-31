--on mainHarness
--  put "a,b,c" into tstr
--  testme @tstr
--  put tstr
--end mainHarness
--
--on testme tstr
--  repeat 3
--    put item 3 of tstr into str3
--    put item 2 of tstr into str2
--    put item 1 of tstr into tstr
--  end repeat
--end testme

on startup
  rgb2hsl @theOutVar,255,0,0
  put theOutVar
end startup

on rgb2hsl outVar,r,g,b
  if number of items of r =  3 then
    put item 1 of ch1/255 into r
    put item 2 of ch1/255 into g
    put item 3 of ch1/255 into b
    put  rgbtohsl(r,g,b) into outvar
  else
    put  rgbtohsl(r/255,g/255,b/255) into outvar
  end if
  return "rgb2hsl <@outvar>, <channel1 value | channelValueList > , [<channel 2 value> , <channel 3 value>]" & cr &  "SuperC! SuperTalk compiler."
end rgb2hsl

function rgbtohsl r,g,b
  put min(r,g,b) into xmin
  put max(r,g,b) into xmax
  put xmax-xmin into xdif
  put (xmax+xmin)/2 into xL
  if xdif = 0 then
    return "0,0," & (xL*100)
  end if

  if xL < 0.5 then
    put xDif/(xMax+xmin) into xS
  else
    put xDif/(2-xMax-Xmin) into xS
  end if

  put (((xMax-r)/6)+(xdif/2))/xdif into rdif
  put (((xMax-g)/6)+(xdif/2))/xdif into gdif
  put (((xMax-b)/6)+(xdif/2))/xdif into bdif
  if r=xmax then
    put bdif-gdif into xH
  else
    if g=xmax then
      put (1/3)+rdif-bdif into xH
    else
      if b=xmax then
        put (2/3)+gdif-rdif into xH
      end if
    end if
  end if
  if xH < 0 then
    add 1 to xh
  end if
  if xh> 1 then
    subtract 1 from xH
  end if
  return xH*360 & "," & xS*100 & "," & xL*100
-- parens around multiplication above avoid 'only multiply numbers' result error string.
end rgbtohsl
