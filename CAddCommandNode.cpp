/*
 *  CAddCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CAddCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"


namespace Carlson
{

void	CAddCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode					*	destValue = GetParamAtIndex( 0 );
	CValueNode					*	srcValue = GetParamAtIndex( 1 );
	CLocalVariableRefValueNode	*	varValue = NULL;
	CFloatValueNode				*	constValue = NULL;
		
	if(( varValue = dynamic_cast<CLocalVariableRefValueNode*>(destValue) ))
	{
		constValue = dynamic_cast<CFloatValueNode*>(srcValue);
		if( constValue )
			inCodeBlock->GenerateAddNumberInstruction( varValue->GetBPRelativeOffset(), constValue->GetAsFloat() );
		else
		{
			CIntValueNode	*	constIntValue = dynamic_cast<CIntValueNode*>(srcValue);
			if( constIntValue )
				inCodeBlock->GenerateAddIntegerInstruction( varValue->GetBPRelativeOffset(), constIntValue->GetAsInt() );
			else
			{
				srcValue->GenerateCode( inCodeBlock );
				
				// TODO: Add code to add non-constant values.
				
				throw std::runtime_error("Can't assign this value.");
			}
		}
	}
	else
		throw std::runtime_error("Can't assign to this value.");
}

} // namespace Carlson