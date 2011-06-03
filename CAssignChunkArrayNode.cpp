/*
 *  CAssignChunkArrayNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CAssignChunkArrayNode.h"
#include "CCodeBlock.h"
#include "CValueNode.h"


namespace Carlson
{

void	CAssignChunkArrayNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CLocalVariableRefValueNode	*	destVar = dynamic_cast<CLocalVariableRefValueNode*>( mParams[0] );
	CIntValueNode	*				chunkType = dynamic_cast<CIntValueNode*>( mParams[1] );
	
	mParams[2]->GenerateCode(inCodeBlock);
	
	inCodeBlock->GenerateAssignChunkArrayInstruction( destVar->GetBPRelativeOffset(), chunkType->GetAsInt() );
}

} // namespace Carlson
