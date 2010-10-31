/*
 *  CFunctionDefinitionNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CFunctionDefinitionNode.h"
#include "CParser.h"


namespace Carlson
{

CFunctionDefinitionNode::~CFunctionDefinitionNode()
{
	std::vector<CFunctionParamVarEntry*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		delete *itty;
		*itty = NULL;
	}
}


void	CFunctionDefinitionNode::AddLocalVar( const std::string& inName,
												const std::string& inUserName,
												TVariantType theType,
												bool initWithName,
												bool isParam,
												bool isGlobal,
												bool dontDispose )
{
	CVariableEntry	newEntry( inUserName, theType, initWithName,
								isParam, isGlobal, dontDispose );
	mLocals[inName] = newEntry;
	if( isGlobal )
		(*mGlobals)[inName] = newEntry;
}


void	CFunctionDefinitionNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Function " << mName << std::endl;
	
	DebugPrintInner( destStream, indentLevel );
}

} /* namespace Carlson */