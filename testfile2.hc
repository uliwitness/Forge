function xIndentScript txt,tabSpaces,selStart,selEnd
  put 0 into indentLevel
  put 0 into lineStartOffset
  put 1 into lineNb
  repeat for each line theLine of txt
    -- get rid of old spaces:
    put countLeadingSpaces(theLine) into oldSpaces
    if oldSpaces > 0 then
      delete char 1 to oldSpaces of theLine
    end if
    
    -- un-indent:
    if word 1 of theLine = "end" or word 1 of theLine = "else" then
      subtract 1 from indentLevel
    end if
    
    -- find out how many spaces we want to indent this line:
    put indentLevel * tabSpaces into newSpaces
    put spaces(newSpaces) before theLine
    
    -- nudge selection
    if lineStartOffset <= selStart-1 then
      add newSpaces -oldSpaces to selStart
    end if
    if lineStartOffset <= selEnd-1 then
      add newSpaces -oldSpaces to selEnd
    end if
    
    -- decide on indentation for following lines:
    if word 1 of theLine = "if" or word 1 of theLine = "repeat" or word 1 of theLine = "on" or word 1 of theLine = "function" then
      add 1 to indentLevel
    else if word 1 of theLine = "else" then
      add 1 to indentLevel -- indent commands following else again.
    end if
    
    -- write out new line and increment counter/offset:
    put theLine into line lineNb of newTxt
    add length(theLine) +1 to lineStartOffset -- +1 for return
    add 1 to lineNb
  end repeat
  return selStart & comma & selEnd & comma & newTxt
end xIndentScript

function countLeadingSpaces str
  put 0 into theCount
  repeat with x = 1 to length(str)
    if char x of str = " " or char x of str = tab then
      add 1 to theCount
    else
      exit repeat
    end if
  end repeat
  return theCount
end countLeadingSpaces

function spaces n
  put "" into str
  repeat with x = 1 to n
    put " " after str
  end repeat
  return str
end spaces