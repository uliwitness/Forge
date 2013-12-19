/*
 *  CGlobalPropertyNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CGlobalPropertyNode.h"
#include "CParseTree.h"
#include "CCodeBlock.h"
#include "CNodeTransformation.h"
#include "LEOInstructions.h"
#include "CForgeExceptions.h"


namespace Carlson
{

void	CGlobalPropertyNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Global Property \"" << gInstructions[mGetterInstructionID].name << "\"" << std::endl
				<< indentChars << "{" << std::endl;
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +1 );
	}
	
	destStream << indentChars << "}" << std::endl;
}


void	CGlobalPropertyNode::AddParam( CValueNode* val )
{
	mParams.push_back( val );
	mParseTree->NodeWasAdded(val);
}


void	CGlobalPropertyNode::Simplify()
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


void	CGlobalPropertyNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	if( mGetterInstructionID == INVALID_INSTR )
	{
		std::string	errMsg("You can only change the ");
		errMsg.append( mPropertyName );
		errMsg.append( " property, not read it." );
		throw CForgeParseError(errMsg,mLineNum);
	}
		
	std::vector<CValueNode*>::iterator itty;
	
	// Push all params on stack:
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
		(*itty)->GenerateCode( inCodeBlock );
	
	inCodeBlock->GenerateOperatorInstruction( mGetterInstructionID );
}


void	CGlobalPropertyNode::GenerateSetterCode( CCodeBlock* inCodeBlock, CValueNode* newValueNode )
{
	if( mSetterInstructionID == INVALID_INSTR )
	{
		std::string	errMsg("You can not change the ");
		errMsg.append( mPropertyName );
		errMsg.append( " property." );
		throw CForgeParseError(errMsg,mLineNum);
	}

	std::vector<CValueNode*>::iterator itty;
	
	// Push all params on stack:
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
		(*itty)->GenerateCode( inCodeBlock );
	
	newValueNode->GenerateCode( inCodeBlock );
	
	inCodeBlock->GenerateOperatorInstruction( mSetterInstructionID );
}

} // namespace Carlson
