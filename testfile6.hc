#!/usr/bin/env Forge
on testx
  global gEventHandlerThing
  
  if gEventHandlerThing is not empty then
    return gEventHandlerThing
  else
    --get InstallEventHandler( GetApplicationEventTarget(), id of handler handleHICommand, 0, nullPointer, nullPointer, nullPointer )
  end if
  
  put [NSColor currentControlTint] into theTint
  put [NSColor colorForControlTint: theTint] into theColor
  put [theColor description]
  
  return 0
end testx

on handleHICommand eventref,callref,refcon
	global gEventHandlerThing
	put "received notification of event" into gEventHandlerThing
end handleHICommand
