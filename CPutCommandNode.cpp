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
#include "CObjectPropertyNode.h"
#include "CGlobalPropertyNode.h"
#include "LEOInstructions.h"
#include <iostream>

namespace Carlson
{

void	CPutCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode					*	destValue = GetParamAtIndex( 1 );
	CValueNode					*	srcValue = GetParamAtIndex( 0 );
	CMakeChunkRefNode			*	chunkValue = NULL;
	CObjectPropertyNode			*	propertyValue = NULL;
	CGlobalPropertyNode			*	globalPropertyValue = NULL;
	
	if(( chunkValue = dynamic_cast<CMakeChunkRefNode*>(destValue) ))
	{
		destValue->GenerateCode( inCodeBlock );
		srcValue->GenerateCode( inCodeBlock );
		
		inCodeBlock->GenerateSetStringInstruction( BACK_OF_STACK );
	}
	else if(( propertyValue = dynamic_cast<CObjectPropertyNode*>(destValue) ))
	{
		std::string		propName;
		propertyValue->GetSymbolName(propName);
		inCodeBlock->GeneratePushIntInstruction( 1, kLEOUnitNone );
		if( propName.length() > 0 )
			inCodeBlock->GeneratePushStringInstruction( propName );
		else
			propertyValue->GetParamAtIndex( 1 )->GenerateCode( inCodeBlock );
		inCodeBlock->GenerateOperatorInstruction( PUSH_ARRAY_CONSTANT_INSTR, 1, 0 );
		propertyValue->GetParamAtIndex( 0 )->GenerateCode( inCodeBlock );
		srcValue->GenerateCode( inCodeBlock );
		
		inCodeBlock->GenerateSetPropertyOfObjectInstruction();
	}
	else if(( globalPropertyValue = dynamic_cast<CGlobalPropertyNode*>(destValue) ))
	{
		globalPropertyValue->GenerateSetterCode( inCodeBlock, srcValue );
	}
	else
	{
		destValue->GenerateCode( inCodeBlock );
		srcValue->GenerateCode( inCodeBlock );
		
		inCodeBlock->GeneratePutValueIntoValueInstruction();

//		DebugPrint( std::cerr, 0 );
//		throw std::runtime_error("Can't assign to this value.");
	}
}

} // namespace Carlson
