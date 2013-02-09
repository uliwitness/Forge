//
//  CGetChunkPropertyNode.cpp
//  Forge
//
//  Created by Uli Kusterer on 03.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "CGetChunkPropertyNode.h"
#include "CCodeBlock.h"


namespace Carlson
{

		
CValueNode*	CGetChunkPropertyNode::Copy()
{
	CGetChunkPropertyNode	*	nodeCopy = new CGetChunkPropertyNode( mParseTree, mLineNum );
	
	std::vector<CValueNode*>::const_iterator	itty;
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		nodeCopy->AddParam( (*itty)->Copy() );
	}
	
	return nodeCopy;

}

void	CGetChunkPropertyNode::GenerateCode( CCodeBlock* inCodeBlock )
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

	inCodeBlock->GeneratePushChunkPropertyInstruction( BACK_OF_STACK, chunkType );
}

} // namespace Carlson
