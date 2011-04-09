/*
 *  CIfNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 19.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CIfNode.h"
#include "CCodeBlock.h"


namespace Carlson
{

/*
	- Push condition
	- Check condition & jump to else section if false -+
		- If section                                   |
	  +-- jump over else section                       |
	  | - else section    <----------------------------+
	  +-->
*/

void	CIfNode::GenerateCode( CCodeBlock* inBlock )
{
	// Push condition:
	mCondition->GenerateCode(inBlock);
	
	// Check condition, jump to Else start if FALSE:
	int32_t	compareInstructionOffset = (int32_t) inBlock->GetNextInstructionOffset();
	inBlock->GenerateJumpRelativeIfFalseInstruction( 0 );
	
	// Generate If section:
	CCodeBlockNode::GenerateCode( inBlock );
	
	// At end of If section, jump *over* Else section:
	int32_t	jumpOverElseInstructionOffset = (int32_t) inBlock->GetNextInstructionOffset();
	inBlock->GenerateJumpRelativeInstruction( 0 );
	
	// Retroactively fill in the address of the Else section in the if's jump instruction:
	int32_t		elseSectionStartOffset = (int32_t) inBlock->GetNextInstructionOffset();
	inBlock->SetJumpAddressOfInstructionAtIndex( compareInstructionOffset, elseSectionStartOffset -compareInstructionOffset );
	
	// Generate Else section:
	mElseBlock->GenerateCode( inBlock );
	
	// Retroactively fill in the address of the end of the Else section in the jump instruction at the If's end:
	int32_t	elseSectionEndOffset = (int32_t) inBlock->GetNextInstructionOffset();
	inBlock->SetJumpAddressOfInstructionAtIndex( jumpOverElseInstructionOffset, elseSectionEndOffset -jumpOverElseInstructionOffset );
}


void	CIfNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "If (" << std::endl;
	mCondition->DebugPrint( destStream, indentLevel );
	destStream << indentChars << ")" << std::endl;
	
	DebugPrintInner( destStream, indentLevel );
	
	destStream << indentChars << "else" << std::endl;
	
	mElseBlock->DebugPrintInner( destStream, indentLevel );
}




}