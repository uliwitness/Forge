//
//  CChunkPropertyNodeTransformation.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 28.05.12.
//  Copyright (c) 2012 Uli Kusterer. All rights reserved.
//

#include "CChunkPropertyNodeTransformation.h"
#include "CMakeChunkConstNode.h"
#include "CGetChunkPropertyNode.h"
#include "CObjectPropertyNode.h"
#include "COperatorNode.h"
#include "LEOInstructions.h"
#include <iostream>


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
			
			So we detect this case here and replace the chunk expression and property expression with a chunk property expression.
		*/
		
		CMakeChunkConstNode	*	constChunkExpr = dynamic_cast<CMakeChunkConstNode*>( inPropNode->GetParamAtIndex(0) );
		if( constChunkExpr != NULL )
		{
			std::string	propName;
			inPropNode->GetSymbolName( propName );
			CGetChunkPropertyNode	*	chunkRefExpr = new CGetChunkPropertyNode( inPropNode->GetParseTree(), propName, constChunkExpr->GetLineNum() );
			for( size_t x = 0; x < constChunkExpr->GetParamCount(); x++ )
			{
				CValueNode	*	theParam = constChunkExpr->GetParamAtIndex(x);
				CValueNode	*	theParamCopy = theParam->Copy();
				chunkRefExpr->AddParam( theParamCopy );
			}
			return chunkRefExpr;
		}
		
		CObjectPropertyNode *	objectPropNode = dynamic_cast<CObjectPropertyNode*>( inPropNode->GetParamAtIndex(0) );
		if( objectPropNode )
		{
			CArrayValueNode * keyPath2 = inPropNode->CopyPropertyNameValue();
			CArrayValueNode * keyPath = objectPropNode->CopyPropertyNameValue();
			for( size_t x = 0; x < keyPath2->GetItemCount(); x++ )
			{
				keyPath->AddItem( keyPath2->GetItem( x ) );
				keyPath2->SetItemAtIndex( NULL, x );	// Make sure it gives up ownership.
			}
			delete keyPath2;
			
			CValueNode * actualTarget = objectPropNode->GetParamAtIndex(0);
			objectPropNode->SetParamAtIndex( 0, NULL );	// Make objectPropNode give up ownership of actualTarget.
			inPropNode->SetParamAtIndex( 0, actualTarget );	// Replace objectPropNode in our target slot with actualTarget.
			inPropNode->SetPropertyNameValue( keyPath );
			
			delete objectPropNode;	// Free no longer needed objectPropNode.
		}
	}
	
	return inPropNode;
}


CNode*	CChunkPropertyPutNodeTransformation::Simplify( CPutCommandNode* inPutNode )
{
	if( inPutNode->GetParamCount() >= 2 )
	{
		/*
			When a chunk property is used as a destination, we replace the "put"
			and "get chunk property" instructions with a "set chunk property"
			instruction.
		*/
		
		CGetChunkPropertyNode	*	chunkExpr = dynamic_cast<CGetChunkPropertyNode*>( inPutNode->GetParamAtIndex(1) );
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
			chunkExpr->GetSymbolName( propName );
			setChunkPropNode->AddParam( new CStringValueNode( inPutNode->GetParseTree(), propName, inPutNode->GetLineNum() ) );
			
			return setChunkPropNode;
		}
	}
	
	return inPutNode;
}


} // namespace Carlson
