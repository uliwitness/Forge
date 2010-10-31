/*
 *  CNode.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include <ostream>


#if 1
#define CNODE_PURE_VIRTUAL	= 0
#else
#define CNODE_PURE_VIRTUAL {}
#endif


namespace Carlson
{

// -----------------------------------------------------------------------------
//	Classes:
// -----------------------------------------------------------------------------

// Abstract root class for things in a parse tree:
//	These are stupid, and can simply be debug-printed or turned into code, and
//	that is it.

class CNode
{
public:
	CNode() {};
	virtual ~CNode() {};
		
	virtual void	DebugPrint( std::ostream& destStream, size_t indentLevel ) = 0;
};


} // namespace Carlson

// -----------------------------------------------------------------------------
//	Helper Macros:
// -----------------------------------------------------------------------------

// The INDENT_PREPARE(level) Macro gives you a char array named 'indentChars'
//	containing the requested number of tabs (or, exceeding an arbitrary limit,
//	just gives you that many tabs and calls it a day). This is used in debug
//	output of the parse tree to indent each level of output dynamically.

#define INDENT_CHARS	{ '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',\
							'\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', 0 }

#define INDENT_PREPARE(indentLevel)	char	indentChars[] = INDENT_CHARS;\
	if( indentLevel < sizeof(indentChars) ) indentChars[indentLevel] = 0

