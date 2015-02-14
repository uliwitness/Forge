/*
 *  CObjectPropertyNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CObjectPropertyNode.h"
#include "CParseTree.h"
#include "CCodeBlock.h"
#include "CMakeChunkConstNode.h"
#include "CMakeChunkRefNode.h"
#include "CNodeTransformation.h"


namespace Carlson
{

CValueNode*	CObjectPropertyNode::Copy()
{
	CObjectPropertyNode	*	nodeCopy = new CObjectPropertyNode( mParseTree, mSymbolName, mLineNum );
	
	std::vector<CValueNode*>::const_iterator	itty;
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		nodeCopy->AddParam( (*itty)->Copy() );
	}
	
	return nodeCopy;
}


void	CObjectPropertyNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Property \"" << mSymbolName << "\"" << std::endl
				<< indentChars << "{" << std::endl;
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +1 );
	}
	
	destStream << indentChars << "}" << std::endl;
}


void	CObjectPropertyNode::AddParam( CValueNode* val )
{
	mParams.push_back( val );
	mParseTree->NodeWasAdded(val);
}


void	CObjectPropertyNode::Simplify()
{
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
		}
	}
	CValueNode::Simplify();
}


void	CObjectPropertyNode::Visit( std::function<void(CNode*)> visitorBlock )
{
	for( auto currParam : mParams )
		currParam->Visit(visitorBlock);
	
	CValueNode::Visit( visitorBlock );
}


void	CObjectPropertyNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	std::vector<CValueNode*>::reverse_iterator itty;
	
	// Push all params on stack (in reverse order!):
	if( mSymbolName.length() != 0 )
		inCodeBlock->GeneratePushStringInstruction( mSymbolName );
	
	for( itty = mParams.rbegin(); itty != mParams.rend(); itty++ )
		(*itty)->GenerateCode( inCodeBlock );
	
	inCodeBlock->GeneratePushPropertyOfObjectInstruction();
}

} // namespace Carlson
