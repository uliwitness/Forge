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
#include "CObjectPropertyNode.h"
#include "COperatorNode.h"
#include "LEOInstructions.h"


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


CNode*	CChunkPropertyPutNodeTransformation::Simplify( CPutCommandNode* inPutNode )
{
	if( inPutNode->GetParamCount() >= 2 )
	{
		/*
			Chunks can have properties as well, in which case we have to tell the
			parent object to modify that chunk of itself, we can't change the
			property once we've evaluated the chunk and turned it into a substring.
			
			So we detect this case here and replace the constant chunk expression
			with a chunk reference.
		*/
		
		CObjectPropertyNode	*	propExpr = dynamic_cast<CObjectPropertyNode*>( inPutNode->GetParamAtIndex(1) );
		if( propExpr )
		{
			CMakeChunkRefNode	*	chunkExpr = dynamic_cast<CMakeChunkRefNode*>( propExpr->GetParamAtIndex(0) );
			if( chunkExpr != NULL )
			{
				CIntValueNode	*	chunkTypeVal = dynamic_cast<CIntValueNode*>(chunkExpr->GetParamAtIndex(1));
				if( !chunkTypeVal )
					return inPutNode;

				COperatorNode	*	setChunkPropNode = new COperatorNode( inPutNode->GetParseTree(), SET_CHUNK_PROPERTY_INSTR, inPutNode->GetLineNum() );
				setChunkPropNode->SetInstructionParams( BACK_OF_STACK, chunkTypeVal->GetAsInt() );
				
				for( size_t x = 0; x < chunkExpr->GetParamCount(); x++ )
				{
					if( x == 1 )
						continue;
					
					CValueNode	*	theParam = chunkExpr->GetParamAtIndex(x);
					CValueNode	*	theParamCopy = theParam->Copy();
					setChunkPropNode->AddParam( theParamCopy );
				}
				
				setChunkPropNode->AddParam( inPutNode->GetParamAtIndex(0)->Copy() );
				
				std::string		propName;
				propExpr->GetSymbolName( propName );
				setChunkPropNode->AddParam( new CStringValueNode( inPutNode->GetParseTree(), propName ) );
				
				return setChunkPropNode;
			}
		}
	}
	
	return inPutNode;
}


} // namespace Carlson