//
//  CConcatOperatorNodeTransformation.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 16.03.2013.
//  Copyright (c) 2013 Uli Kusterer. All rights reserved.
//

#include "CConcatOperatorNodeTransformation.h"
#include "LEOInstructions.h"


namespace Carlson
{

LEOInstructionID	CConcatOperatorNodeTransformation::GetInstructionID()
{
	return CONCATENATE_VALUES_INSTR;
}

CNode*	CConcatOperatorNodeTransformation::Simplify( COperatorNode* inOperatorNode )
{
	if( inOperatorNode->GetParamCount() >= 2 )
	{
		CValueNode	*	firstParam = dynamic_cast<CValueNode*>( inOperatorNode->GetParamAtIndex(0) );
		CValueNode	*	secondParam = dynamic_cast<CValueNode*>( inOperatorNode->GetParamAtIndex(1) );
		if( firstParam && secondParam && firstParam->IsConstant() && secondParam->IsConstant() )
		{
			std::string	firstParamStr = firstParam->GetAsString();
			std::string	secondParamStr = secondParam->GetAsString();
			std::string	newString = firstParamStr;
			newString.append( secondParamStr );
			return new CStringValueNode( inOperatorNode->GetParseTree(), newString );
		}
	}
	
	return inOperatorNode;
}


} // namespace Carlson