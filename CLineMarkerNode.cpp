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

void	CLineMarkerNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "# LINE " << mLineNum << std::endl;
}

void	CLineMarkerNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GenerateLineMarkerInstruction( (uint32_t) mLineNum );
}

} // namespace Carlson
