/*
 *  CCodeBlockNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CCodeBlockNode.h"
#include "CParser.h"

namespace Carlson
{

void	CCodeBlockNodeBase::GenerateCode( CodeBlock& codeBlock )
{
	std::vector<CNode*>::iterator	itty;
	
	for( itty = mCommands.begin(); itty != mCommands.end(); itty++ )
	{
		(*itty)->GenerateCode( codeBlock );
	}
}


void	CCodeBlockNodeBase::GenerateCpp( CppBlock& codeBlock )
{
	std::vector<CNode*>::iterator	itty;
	
	for( itty = mCommands.begin(); itty != mCommands.end(); itty++ )
	{
		(*itty)->GenerateCpp( codeBlock );
	}
}


void	CCodeBlockNodeBase::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Code Block" << std::endl
				<< indentChars << "{" << std::endl
				<< indentChars << "\tcommands" << std::endl
				<< indentChars << "\t{" << std::endl;
	
	std::vector<CNode*>::iterator itty;
	
	for( itty = mCommands.begin(); itty != mCommands.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +2 );
	}
	
	destStream << indentChars << "\t}" << std::endl;
	
	destStream << indentChars << "}" << std::endl;
}

CCodeBlockNodeBase::~CCodeBlockNodeBase()
{
	std::vector<CNode*>::iterator itty;
	
	for( itty = mCommands.begin(); itty != mCommands.end(); itty++ )
	{
		delete *itty;
		*itty = NULL;
	}
}


void	CCodeBlockNode::AddLocalVar( const std::string& inName, const std::string& inUserName,
								TVariantType theType, bool initWithName,
								bool isParam, bool isGlobal,
								bool dontDispose )
{
	CVariableEntry	newEntry( inUserName, theType, initWithName,
								isParam, isGlobal, dontDispose );
	(*mLocals)[inName] = newEntry;
	if( isGlobal )
		(*mGlobals)[inName] = newEntry;
}


} // namespace Carlson