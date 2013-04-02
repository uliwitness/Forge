/*
 *  CDivideCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CDivideCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"
#include "LEOInstructions.h"


namespace Carlson
{

void	CDivideCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode					*	destValue = GetParamAtIndex( 0 );
	CValueNode					*	srcValue = GetParamAtIndex( 1 );
		
	srcValue->GenerateCode( inCodeBlock );
	destValue->GenerateCode( inCodeBlock );
	inCodeBlock->GenerateOperatorInstruction( DIVIDE_COMMAND_INSTR );
}

} // namespace Carlson
