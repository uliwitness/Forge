/*
 *  CGetArrayItemCountNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CGetArrayItemCountNode.h"
#include "CCodeBlock.h"
#include "CValueNode.h"


namespace Carlson
{

void	CGetArrayItemCountNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CLocalVariableRefValueNode	*	destVar = dynamic_cast<CLocalVariableRefValueNode*>( mParams[0] );
	
	mParams[1]->GenerateCode(inCodeBlock);
	
	inCodeBlock->GenerateGetArrayItemCountInstruction( destVar->GetBPRelativeOffset() );
}

} // namespace Carlson