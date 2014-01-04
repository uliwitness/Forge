/*
 *  CParseErrorCommandNode.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CParseErrorCommandNode.h"
#include "CValueNode.h"
#include "CCodeBlock.h"

namespace Carlson
{

void	CParseErrorCommandNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GenerateParseErrorInstruction( mErrorMessage, mFileName, mLineNum, mOffset );
}

} // namespace Carlson
