/*
 *  CParseTree.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CParseTree.h"

namespace Carlson
{

CParseTree::~CParseTree()
{
	std::deque<CNode*>::iterator itty;
	
	for( itty = mNodes.begin(); itty != mNodes.end(); itty++ )
	{
		delete *itty;
		*itty = NULL;
	}
}


void	CParseTree::GenerateCode( CCodeBlock* inCodeBlock )
{
	std::deque<CNode*>::iterator itty;
	
	for( itty = mNodes.begin(); itty != mNodes.end(); itty++ )
	{
		(*itty)->GenerateCode( inCodeBlock );
	}
}


void	CParseTree::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Parse Tree" << std::endl
				<< indentChars << "{" << std::endl;
	
	std::deque<CNode*>::iterator itty;
	
	for( itty = mNodes.begin(); itty != mNodes.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +1 );
	}
	
	destStream << indentChars << "}" << std::endl;
}


} // namespace Carlson