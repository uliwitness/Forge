//
//  CMakeChunkRefNode.cpp
//  Forge
//
//  Created by Uli Kusterer on 03.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "CMakeChunkRefNode.h"
#include "CCodeBlock.h"


namespace Carlson
{

		
CValueNode*	CMakeChunkRefNode::Copy()
{
	CMakeChunkRefNode	*	nodeCopy = new CMakeChunkRefNode( mParseTree, mLineNum );
	
	std::vector<CValueNode*>::const_iterator	itty;
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		nodeCopy->AddParam( (*itty)->Copy() );
	}
	
	return nodeCopy;
}

void	CMakeChunkRefNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	std::vector<CValueNode*>::const_iterator	itty = mParams.begin();
	int16_t										bpRelativeOffset = BACK_OF_STACK;	// Means
	
	CLocalVariableRefValueNode * theVar = dynamic_cast<CLocalVariableRefValueNode*>(*itty);
	
	if( theVar )
		bpRelativeOffset = theVar->GetBPRelativeOffset();
	else
	{
		(*itty)->GenerateCode( inCodeBlock );
		bpRelativeOffset = BACK_OF_STACK;
	}
	
	itty++;
	
	uint32_t chunkType = (*itty)->GetAsInt();
	
	itty++;
	
	for( ; itty != mParams.end(); itty++ )
	{
		(*itty)->GenerateCode( inCodeBlock );
	}

	inCodeBlock->GeneratePushChunkRefInstruction( bpRelativeOffset, chunkType );
}

} // namespace Carlson
