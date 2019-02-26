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


void	CParseErrorCommandNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Command \"" << mSymbolName << "\"" << std::endl
				<< indentChars << "{" << std::endl;
	
	destStream << indentChars << "\t" << mFileName << ":" << mLineNum;
	if( mOffset != SIZE_MAX )
		destStream << ":" << mOffset;
	destStream << ": " << mErrorMessage << std::endl;
		
	destStream << indentChars << "}" << std::endl;
}


} // namespace Carlson
