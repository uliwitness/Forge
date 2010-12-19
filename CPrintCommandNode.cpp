/*
 *  CPrintCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CPrintCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"

namespace Carlson
{

void	CPrintCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	GetParamAtIndex( 0 )->GenerateCode( inCodeBlock );
	
	inCodeBlock->GeneratePrintValueInstruction();
}

} // namespace Carlson