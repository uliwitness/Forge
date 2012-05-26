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


void	CNodeTransformationBase::Apply( CNode* inNode )
{
	std::vector<CNodeTransformationBase*>::const_iterator	itty;
	for( itty = sNodeTransformations.begin(); itty != sNodeTransformations.end(); itty++ )
	{
		(*itty)->Simplify_External( inNode );
	}
}

}