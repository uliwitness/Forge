//
//  CConcatSpaceOperatorNodeTransformation.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 16.03.2013.
//  Copyright (c) 2013 Uli Kusterer. All rights reserved.
//

#include "CConcatSpaceOperatorNodeTransformation.h"
#include "LEOInstructions.h"


namespace Carlson
{

LEOInstructionID	CConcatSpaceOperatorNodeTransformation::GetInstructionID()
{
	return CONCATENATE_VALUES_WITH_SPACE_INSTR;
}

CNode*	CConcatSpaceOperatorNodeTransformation::Simplify( COperatorNode* inOperatorNode )
{
	if( inOperatorNode->GetParamCount() == 2 )
	{
		CValueNode	*	firstParam = dynamic_cast<CValueNode*>( inOperatorNode->GetParamAtIndex(0) );
		CValueNode	*	secondParam = dynamic_cast<CValueNode*>( inOperatorNode->GetParamAtIndex(1) );
		if( firstParam && secondParam )
		{
			if( firstParam->IsConstant() && secondParam->IsConstant() )	// Fold the entire expression in a constant:
			{
				std::string	firstParamStr = firstParam->GetAsString();
				std::string	secondParamStr = secondParam->GetAsString();
				std::string	newString = firstParamStr;
				newString.append( 1, ' ' );
				newString.append( secondParamStr );
				return new CStringValueNode( inOperatorNode->GetParseTree(), newString, inOperatorNode->GetLineNum() );
			}
			else if( firstParam->IsConstant() )	// Fold the space into the leading constant:
			{
				std::string	firstParamStr = firstParam->GetAsString();
				std::string	newString = firstParamStr;
				newString.append( 1, ' ' );
				CStringValueNode	*	newFirstParamNode = new CStringValueNode( inOperatorNode->GetParseTree(), newString, firstParam->GetLineNum() );
				CValueNode			*	newSecondParamNode = secondParam->Copy();
				COperatorNode		*	newOperationNode = new COperatorNode( inOperatorNode->GetParseTree(), CONCATENATE_VALUES_INSTR, inOperatorNode->GetLineNum() );
				newOperationNode->AddParam( newFirstParamNode );
				newOperationNode->AddParam( newSecondParamNode );
				return newOperationNode;
			}
			else if( secondParam->IsConstant() )	// Fold the space into the trailing constant:
			{
				std::string	secondParamStr = secondParam->GetAsString();
				std::string	newString( " " );
				newString.append( secondParamStr );
				CStringValueNode	*	newSecondParamNode = new CStringValueNode( inOperatorNode->GetParseTree(), newString, secondParam->GetLineNum() );
				CValueNode			*	newFirstParamNode = firstParam->Copy();
				COperatorNode		*	newOperationNode = new COperatorNode( inOperatorNode->GetParseTree(), CONCATENATE_VALUES_INSTR, inOperatorNode->GetLineNum() );
				newOperationNode->AddParam( newFirstParamNode );
				newOperationNode->AddParam( newSecondParamNode );
				return newOperationNode;
			}
		}
	}
	
	return inOperatorNode;
}


} // namespace Carlson