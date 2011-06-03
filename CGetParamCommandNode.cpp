/*
 *  CGetParamCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CGetParamCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"

namespace Carlson
{

void	CGetParamCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode					*	destValue = GetParamAtIndex( 0 );
	CValueNode					*	paramIdx = GetParamAtIndex( 1 );
	CLocalVariableRefValueNode	*	varValue = NULL;
	
	if(( varValue = dynamic_cast<CLocalVariableRefValueNode*>(destValue) ))
		inCodeBlock->GenerateAssignParamToVariableInstruction( varValue->GetBPRelativeOffset(), paramIdx->GetAsInt() );
	else
		throw std::runtime_error("Can't assign to this value.");
}

} // namespace Carlson
