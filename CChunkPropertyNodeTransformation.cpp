//
//  CChunkPropertyNodeTransformation.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 28.05.12.
//  Copyright (c) 2012 Uli Kusterer. All rights reserved.
//

#include "CChunkPropertyNodeTransformation.h"
#include "CMakeChunkConstNode.h"
#include "CMakeChunkRefNode.h"


namespace Carlson
{


CNode*	CChunkPropertyNodeTransformation::Simplify( CObjectPropertyNode* inPropNode )
{
	if( inPropNode->GetParamCount() >= 1 )
	{
		/*
			Chunks can have properties as well, in which case we have to tell the
			parent object to modify that chunk of itself, we can't change the
			property once we've evaluated the chunk and turned it into a substring.
			
			So we detect this case here and replace the constant chunk expression
			with a chunk reference.
		*/
		
		CMakeChunkConstNode	*	constChunkExpr = dynamic_cast<CMakeChunkConstNode*>( inPropNode->GetParamAtIndex(0) );
		if( constChunkExpr != NULL )
		{
			CMakeChunkRefNode	*	chunkRefExpr = new CMakeChunkRefNode( inPropNode->GetParseTree(), constChunkExpr->GetLineNum() );
			for( size_t x = 0; x < constChunkExpr->GetParamCount(); x++ )
			{
				CValueNode	*	theParam = constChunkExpr->GetParamAtIndex(x);
				CValueNode	*	theParamCopy = theParam->Copy();
				chunkRefExpr->AddParam( theParamCopy );
			}
			delete inPropNode->GetParamAtIndex(0);
			inPropNode->SetParamAtIndex(0, chunkRefExpr);
		}
	}
	
	return inPropNode;
}


} // namespace Carlson