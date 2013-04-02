/*
 *  CSubtractCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CSubtractCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"
#include "LEOInstructions.h"


namespace Carlson
{

void	CSubtractCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode					*	destValue = GetParamAtIndex( 1 );
	CValueNode					*	srcValue = GetParamAtIndex( 0 );
		
	srcValue->GenerateCode( inCodeBlock );
	destValue->GenerateCode( inCodeBlock );
	inCodeBlock->GenerateOperatorInstruction( SUBTRACT_COMMAND_INSTR );
}

} // namespace Carlson
