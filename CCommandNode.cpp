/*
 *  CCommandNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"
#include "CValueNode.h"
#include "CParseTree.h"
#include <assert.h>
#include "CNodeTransformation.h"


namespace Carlson
{

void	CCommandNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Command \"" << mSymbolName << "\"" << std::endl
				<< indentChars << "{" << std::endl;
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +1 );
	}
		
	destStream << indentChars << "}" << std::endl;
}


void	CCommandNode::AddParam( CValueNode* val )
{
	mParams.push_back( val );
	mParseTree->NodeWasAdded(val);
}


void	CCommandNode::Simplify()
{
	CNode::Simplify();
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		CValueNode	*	originalNode = *itty;
		originalNode->Simplify();	// Give subnodes a chance to apply transformations first. Might expose simpler sub-nodes we can then simplify.
		CNode* newNode = CNodeTransformationBase::Apply( originalNode );	// Returns either originalNode, or a totally new object, in which case we delete the old one.
		if( newNode != originalNode )
		{
			assert( dynamic_cast<CValueNode*>(newNode) != NULL );
			*itty = (CValueNode*)newNode;
			delete originalNode;
		}
	}
}


void	CCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->GenerateCode( inCodeBlock );
	}
}


} // namespace Carlson
