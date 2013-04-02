/*
 *  CMultiplyCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CMultiplyCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"
#include "LEOInstructions.h"


namespace Carlson
{

void	CMultiplyCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode					*	destValue = GetParamAtIndex( 0 );
	CValueNode					*	srcValue = GetParamAtIndex( 1 );
		
	destValue->GenerateCode( inCodeBlock );
	srcValue->GenerateCode( inCodeBlock );
	inCodeBlock->GenerateOperatorInstruction( MULTIPLY_COMMAND_INSTR );
}

} // namespace Carlson
