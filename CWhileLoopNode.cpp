/*
 *  CWhileLoopNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 19.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CWhileLoopNode.h"
#include "CCodeBlock.h"
#include "CNodeTransformation.h"


namespace Carlson
{

void	CWhileLoopNode::GenerateCode( CCodeBlock* inBlock )
{
	int32_t	lineMarkerInstructionOffset = (int32_t) inBlock->GetNextInstructionOffset();
	inBlock->GenerateLineMarkerInstruction( (int32_t) mLineNum );	// Make sure debugger indicates condition as current line on every iteration.
	
	// Push condition:
	mCondition->GenerateCode(inBlock);
	
	// Check condition, jump to end of loop if FALSE:
	int32_t	compareInstructionOffset = (int32_t) inBlock->GetNextInstructionOffset();
	inBlock->GenerateJumpRelativeIfFalseInstruction( 0 );
	
	// Generate loop commands:
	CCodeBlockNode::GenerateCode( inBlock );
	
	// At end of loop section, jump back to compare instruction:
	int32_t	jumpBackInstructionOffset = (int32_t) inBlock->GetNextInstructionOffset();
	inBlock->GenerateJumpRelativeInstruction( lineMarkerInstructionOffset -jumpBackInstructionOffset );
	
	// Retroactively fill in the address of the Else section in the if's jump instruction:
	int32_t	loopEndOffset = (int32_t) inBlock->GetNextInstructionOffset();
	inBlock->SetJumpAddressOfInstructionAtIndex( compareInstructionOffset, loopEndOffset -compareInstructionOffset );
}


void	CWhileLoopNode::Simplify()
{
	CValueNode	*	originalNode = mCondition;
	originalNode->Simplify();	// Give subnodes a chance to apply transformations first. Might expose simpler sub-nodes we can then simplify.
	CNode* newNode = CNodeTransformationBase::Apply( originalNode );	// Returns either originalNode, or a totally new object, in which case we delete the old one.
	if( newNode != originalNode )
	{
		assert( dynamic_cast<CValueNode*>(newNode) != NULL );
		mCondition = (CValueNode*)newNode;
	}

	CCodeBlockNode::Simplify();
}


void	CWhileLoopNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "While" << std::endl << indentChars << "(" << std::endl;
	mCondition->DebugPrint( destStream, indentLevel +1 );
	destStream << indentChars << ")" << std::endl;
	
	DebugPrintInner( destStream, indentLevel );
}

} /*Carlson*/
