/*
 *  CFunctionCallNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CFunctionCallNode.h"
#include "CParseTree.h"


namespace Carlson
{

void	CFunctionCallNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Function Call \"" << mSymbolName << "\"" << std::endl
				<< indentChars << "{" << std::endl;
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +1 );
	}
	
	destStream << indentChars << "}" << std::endl;
}


void	CFunctionCallNode::AddParam( CValueNode* val )
{
	mParams.push_back( val );
	mParseTree->NodeWasAdded(val);
}

} // namespace Carlson