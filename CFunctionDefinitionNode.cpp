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

void	CFunctionDefinitionNode::GenerateCode( CodeBlock& codeBlock )
{
	codeBlock.export_method( mName );
	
	LocalVarEntryMap		varSlots;
	
	std::map<std::string,CVariableEntry>::iterator	localsItty;
	for( localsItty = mLocals.begin(); localsItty != mLocals.end(); localsItty++ )
	{
		varSlots[ localsItty->first ] = LocalVarEntry( 4 );
	}
	
	codeBlock.push_back_function_prolog( varSlots );
	
	CCodeBlockNodeBase::GenerateCode( codeBlock );
	
	codeBlock.push_back_function_epilog();
}


void	CFunctionDefinitionNode::GenerateCpp( CppBlock& codeBlock )
{
	codeBlock.generate_function_definition_start( mName );
	
	std::map<std::string,CVariableEntry>::iterator	localsItty;
	for( localsItty = mLocals.begin(); localsItty != mLocals.end(); localsItty++ )
	{
		codeBlock.declare_local_var( localsItty->first );
	}
	
	CCodeBlockNodeBase::GenerateCpp( codeBlock );
	
	codeBlock.generate_function_definition_end( mName );
}


} /* namespace Carlson */