/*
 *  CCommandNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"
#include "CValueNode.h"


namespace Carlson
{

void	CCommandNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Command \"" << mSymbolName << "\"" << std::endl
				<< indentChars << "{" << std::endl
				<< indentChars << "\tparams" << std::endl
				<< indentChars << "\t{" << std::endl;
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +2 );
	}
	
	destStream << indentChars << "\t}" << std::endl;
	
	destStream << indentChars << "}" << std::endl;
}

} // namespace Carlson