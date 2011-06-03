/*
 *  CAssignCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CAssignCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"

namespace Carlson
{

void	CAssignCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode					*	destValue = GetParamAtIndex( 0 );
	CValueNode					*	srcValue = GetParamAtIndex( 1 );
	CLocalVariableRefValueNode	*	varValue = NULL;
	
	srcValue->GenerateCode( inCodeBlock );
	
	if(( varValue = dynamic_cast<CLocalVariableRefValueNode*>(destValue) ))
		inCodeBlock->GeneratePopIntoVariableInstruction( varValue->GetBPRelativeOffset() );
	else
		throw std::runtime_error("Can't assign to this value.");
}

} // namespace Carlson
