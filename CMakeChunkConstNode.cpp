//
//  CMakeChunkConstNode.cpp
//  Forge
//
//  Created by Uli Kusterer on 03.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "CMakeChunkConstNode.h"
#include "CCodeBlock.h"


namespace Carlson
{

		
CValueNode*	CMakeChunkConstNode::Copy()
{
	CMakeChunkConstNode	*	nodeCopy = new CMakeChunkConstNode( mParseTree, mLineNum );
	
	std::vector<CValueNode*>::const_iterator	itty;
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		nodeCopy->AddParam( (*itty)->Copy() );
	}
	
	return nodeCopy;

}

void	CMakeChunkConstNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	std::vector<CValueNode*>::const_iterator	itty = mParams.begin();
	
	(*itty)->GenerateCode( inCodeBlock );
	
	itty++;
	
	uint32_t chunkType = (*itty)->GetAsInt();
	
	itty++;
	
	for( ; itty != mParams.end(); itty++ )
	{
		(*itty)->GenerateCode( inCodeBlock );
	}

	#if 0
	// We want to use GeneratePushChunkConstInstruction here, but for properties
	//	we will need a chunk reference instead. +++ TODO: build non-const syntax
	//	tree element if we see it's used by a property.
	inCodeBlock->GeneratePushChunkRefInstruction( BACK_OF_STACK, chunkType );
	#else
	inCodeBlock->GeneratePushChunkConstInstruction( BACK_OF_STACK, chunkType );
	#endif
}

} // namespace Carlson
