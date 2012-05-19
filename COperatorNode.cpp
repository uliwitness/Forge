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


namespace Carlson
{

void	COperatorNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Operator Call \"" << gInstructionNames[mInstructionID] << "\"" << std::endl
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
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
		(*itty)->Simplify();
	
	switch( mInstructionID )
	{
		case CONCATENATE_VALUES_INSTR:
			
			break;
		
		case CONCATENATE_VALUES_WITH_SPACE_INSTR:
			
			break;
	}
}


void	COperatorNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	std::vector<CValueNode*>::iterator itty;
	
	// Push all params on stack:
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
		(*itty)->GenerateCode( inCodeBlock );
	
	inCodeBlock->GenerateOperatorInstruction( mInstructionID );
}

} // namespace Carlson
