/*
 *  COperatorNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "COperatorNode.h"
#include "CParseTree.h"
#include "CCodeBlock.h"
#include "LEOInstructions.h"
#include "CNodeTransformation.h"


namespace Carlson
{

void	COperatorNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << GetDebugNodeName() << " \"" << ((gNumInstructions > mInstructionID) ? gInstructionNames[mInstructionID] : "???") << "\"" << std::endl
				<< indentChars << "{" << std::endl;
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +1 );
	}
	
	destStream << indentChars << "}" << std::endl;
}


void	COperatorNode::AddParam( CValueNode* val )
{
	mParams.push_back( val );
	mParseTree->NodeWasAdded(val);
}


CValueNode*	COperatorNode::Copy()
{
	COperatorNode	*	nodeCopy = new COperatorNode( mParseTree, mInstructionID, mLineNum );
	
	std::vector<CValueNode*>::const_iterator	itty;
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		nodeCopy->AddParam( (*itty)->Copy() );
	}
	
	return nodeCopy;
}


void	COperatorNode::Simplify()
{
	CValueNode::Simplify();
	
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
}


void	COperatorNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	std::vector<CValueNode*>::iterator itty;
	
	// Push all params on stack:
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
		(*itty)->GenerateCode( inCodeBlock );
	
	inCodeBlock->GenerateOperatorInstruction( mInstructionID, mInstructionParam1, mInstructionParam2 );
}

} // namespace Carlson
