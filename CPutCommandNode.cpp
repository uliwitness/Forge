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
#include "CMakeChunkRefNode.h"
#include <iostream>

namespace Carlson
{

void	CPutCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode					*	destValue = GetParamAtIndex( 1 );
	CValueNode					*	srcValue = GetParamAtIndex( 0 );
	CLocalVariableRefValueNode	*	varValue = NULL;
	CMakeChunkRefNode			*	chunkValue = NULL;
	
	if(( varValue = dynamic_cast<CLocalVariableRefValueNode*>(destValue) ))
	{
		srcValue->GenerateCode( inCodeBlock );
		
		inCodeBlock->GeneratePopSimpleValueIntoVariableInstruction( varValue->GetBPRelativeOffset() );
	}
	else if(( chunkValue = dynamic_cast<CMakeChunkRefNode*>(destValue) ))
	{
		destValue->GenerateCode( inCodeBlock );
		srcValue->GenerateCode( inCodeBlock );
		
		inCodeBlock->GenerateSetStringInstruction( BACK_OF_STACK );
	}
	else
	{
		DebugPrint( std::cerr, 0 );
		throw std::runtime_error("Can't assign to this value.");
	}
}

} // namespace Carlson