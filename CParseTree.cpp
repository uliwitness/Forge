/*
 *  CParseTree.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CParseTree.h"
#include "CNodeTransformation.h"

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


void	CParseTree::Simplify()
{
	std::deque<CNode*>::iterator itty;
	
	for( itty = mNodes.begin(); itty != mNodes.end(); itty++ )
	{
		CNode	*	originalNode = *itty;
		originalNode->Simplify();	// Give subnodes a chance to apply transformations first. Might expose simpler sub-nodes we can then simplify.
		CNode* newNode = CNodeTransformationBase::Apply( originalNode );	// Returns either originalNode, or a totally new object, in which case we delete the old one.
		if( newNode != originalNode )
		{
			*itty = newNode;
			delete originalNode;
		}
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
	
	std::deque<CNode*>::iterator itty;
	
	for( itty = mNodes.begin(); itty != mNodes.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel );
	}
}


} // namespace Carlson
