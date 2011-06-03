/*
 *  CPushValueCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CPushValueCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"

namespace Carlson
{

void	CPushValueCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	CValueNode		*	theValue = GetParamAtIndex( 0 );
	theValue->GenerateCode( inCodeBlock );
}

} // namespace Carlson
