//
//  CNodeTransformation.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 26.05.12.
//  Copyright (c) 2012 Uli Kusterer. All rights reserved.
//

#include "CNodeTransformation.h"


namespace Carlson
{

std::vector<CNodeTransformationBase*>		sNodeTransformations;


CNode*	CNodeTransformationBase::Apply( CNode* inNode )
{
	CNode	*	currNode = inNode;
	std::vector<CNodeTransformationBase*>::const_iterator	itty;
	for( itty = sNodeTransformations.begin(); itty != sNodeTransformations.end(); itty++ )
	{
		CNode	*	originalNode = currNode;
		currNode = (*itty)->Simplify_External( originalNode );
		if( currNode != originalNode )
			delete originalNode;
	}
	
	return currNode;
}

}