//
//  COperatorNodeTransformation.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 16.03.2013.
//  Copyright (c) 2013 Uli Kusterer. All rights reserved.
//

#include "COperatorNodeTransformation.h"


namespace Carlson
{

CNode*	COperatorNodeTransformation::Simplify_External( CNode* inNode )
{
	COperatorNode * subclassPtr = dynamic_cast<COperatorNode*>(inNode);
	
	if( subclassPtr )
	{
		if( subclassPtr->GetInstructionID() == GetInstructionID() )
			return Simplify( subclassPtr );
	}
	
	return inNode;
}

} // namespace Carlson