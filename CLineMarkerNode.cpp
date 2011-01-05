/*
 *  CLineMarkerNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CLineMarkerNode.h"
#include "CCodeBlock.h"


namespace Carlson
{

void	CLineMarkerNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GenerateLineMarkerInstruction( mLineNum );
}

} // namespace Carlson