/*
 *  CGetArrayItemNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CGetArrayItemNode.h"
#include "CCodeBlock.h"
#include "CValueNode.h"


namespace Carlson
{

void	CGetArrayItemNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CLocalVariableRefValueNode	*	destVar = dynamic_cast<CLocalVariableRefValueNode*>( mParams[0] );
	
	mParams[1]->GenerateCode(inCodeBlock);
	mParams[2]->GenerateCode(inCodeBlock);
	
	inCodeBlock->GenerateGetArrayItemInstruction( destVar->GetBPRelativeOffset() );
}

} // namespace Carlson
