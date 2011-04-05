on testx
  put "hey,there,kill,violence,now" into theList
  --delete item 5 of theList
  --delete item 1 of theList
  --delete item 2 of theList
  put "is," before item 2 of theList
  put ",no" after item 2 of theList
  
  put theList & return
  
  put itemDelimiter into saveDelim
  
  set the itemDelim to "."
  put (item 2 of "192.168.0.1") &return
  
  set the itemDel to saveDelim
  
  return 0
end testx