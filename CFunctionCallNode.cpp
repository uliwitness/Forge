/*
 *  CFunctionCallNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CFunctionCallNode.h"
#include "CParseTree.h"
#include "CCodeBlock.h"
#include "CNodeTransformation.h"
#include "LEOInstructions.h"


namespace Carlson
{

CValueNode*	CFunctionCallNode::Copy()
{
	CFunctionCallNode	*	nodeCopy = new CFunctionCallNode( mParseTree, mIsCommand, mSymbolName, mLineNum );
	
	std::vector<CValueNode*>::const_iterator	itty;
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		nodeCopy->AddParam( (*itty)->Copy() );
	}
	
	return nodeCopy;
}


void	CFunctionCallNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << GetNodeName() << " \"" << mSymbolName << "\"" << std::endl
				<< indentChars << "{" << std::endl;
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +1 );
	}
	
	destStream << indentChars << "}" << std::endl;
}


void	CFunctionCallNode::AddParam( CValueNode* val )
{
	mParams.push_back( val );
	mParseTree->NodeWasAdded(val);
}


void	CFunctionCallNode::Simplify()
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


void	CFunctionCallNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	LEOInstructionID	instructionID = INVALID_INSTR;
	
	if( mSymbolName.compare( "numtochar" ) == 0 )
		instructionID = NUM_TO_CHAR_INSTR;
	else if( mSymbolName.compare( "chartonum" ) == 0 )
		instructionID = CHAR_TO_NUM_INSTR;
	else if( mSymbolName.compare( "numtohex" ) == 0 )
		instructionID = NUM_TO_HEX_INSTR;
	else if( mSymbolName.compare( "hextonum" ) == 0 )
		instructionID = HEX_TO_NUM_INSTR;
	else if( mSymbolName.compare( "numtobinary" ) == 0 )
		instructionID = NUM_TO_BINARY_INSTR;
	else if( mSymbolName.compare( "binarytonum" ) == 0 )
		instructionID = BINARY_TO_NUM_INSTR;
	else if( mSymbolName.compare( "paramcount" ) == 0 )
		instructionID = PARAMETER_COUNT_INSTR;
	
	if( instructionID != INVALID_INSTR )
	{
		// Push the param on the stack:
		if( mParams.size() > 0 )
			mParams[0]->GenerateCode( inCodeBlock );
		
		inCodeBlock->GenerateOperatorInstruction( instructionID );
	}
	else
	{
		std::vector<CValueNode*>::reverse_iterator itty;
		
		inCodeBlock->GeneratePushStringInstruction( "" );	// Reserve space for the result.
		
		// Push all params on stack (in reverse order!):
		for( itty = mParams.rbegin(); itty != mParams.rend(); itty++ )
			(*itty)->GenerateCode( inCodeBlock );
		
		size_t		numParams = mParams.size();
		inCodeBlock->GeneratePushIntInstruction( (int)numParams, kLEOUnitNone );
		
		// *** Call ***
		inCodeBlock->GenerateFunctionCallInstruction( mIsCommand, mIsMessagePassing, mSymbolName );
		
		// Clean up param count:
		inCodeBlock->GeneratePopValueInstruction();
		
		// Clean up params:
		for( size_t x = 0; x < numParams; x++ )
			inCodeBlock->GeneratePopValueInstruction();
		
		// We leave the result on the stack.
	}
}

} // namespace Carlson
