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


namespace Carlson
{

void	CWhileLoopNode::GenerateCode( CCodeBlock* inBlock )
{
	size_t	lineMarkerInstructionOffset = inBlock->GetNextInstructionOffset();
	inBlock->GenerateLineMarkerInstruction( mLineNum );	// Make sure debugger indicates condition as current line on every iteration.
	
	// Push condition:
	mCondition->GenerateCode(inBlock);
	
	// Check condition, jump to end of loop if FALSE:
	size_t	compareInstructionOffset = inBlock->GetNextInstructionOffset();
	inBlock->GenerateJumpRelativeIfFalseInstruction( 0 );
	
	// Generate loop commands:
	CCodeBlockNode::GenerateCode( inBlock );
	
	// At end of loop section, jump back to compare instruction:
	size_t	jumpBackInstructionOffset = inBlock->GetNextInstructionOffset();
	inBlock->GenerateJumpRelativeInstruction( lineMarkerInstructionOffset -jumpBackInstructionOffset );
	
	// Retroactively fill in the address of the Else section in the if's jump instruction:
	size_t	loopEndOffset = inBlock->GetNextInstructionOffset();
	inBlock->SetJumpAddressOfInstructionAtIndex( compareInstructionOffset, loopEndOffset -compareInstructionOffset );
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