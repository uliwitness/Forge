/*
 *  CPutCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CPutCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"

namespace Carlson
{

void	CPutCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode					*	destValue = GetParamAtIndex( 1 );
	CValueNode					*	srcValue = GetParamAtIndex( 0 );
	CLocalVariableRefValueNode	*	varValue = NULL;
	
	srcValue->GenerateCode( inCodeBlock );
	
	if(( varValue = dynamic_cast<CLocalVariableRefValueNode*>(destValue) ))
		inCodeBlock->GeneratePopIntoVariableInstruction( varValue->GetBPRelativeOffset() );
	else
		throw std::runtime_error("Can't assign to this value.");
}

} // namespace Carlson