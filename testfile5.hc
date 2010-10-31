on testx
  put "hey,there,kill,violence,now" into theList
  delete item 5 of theList
  delete item 1 of theList
  delete item 2 of theList
  put "is," before item 2 of theList
  put ",no" after item 2 of theList
  
  put theList
  
  set the itemDelim to "."
  put item 2 of "192.168.0.1"
  
  return 0
end testx