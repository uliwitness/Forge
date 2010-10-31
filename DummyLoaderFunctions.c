/*
 *  DummyLoaderFunctions.c
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 02.12.07.
 *  Copyright 2007 The Void Software. All rights reserved.
 *
 */

#include <memory.h>

void*	LookUpSymbol( const char* inSymbolName, unsigned char foundInstruction )
{
	return (void*) &memcpy;
}