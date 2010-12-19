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
#include "CCodeBlock.h"


namespace Carlson
{

CFunctionDefinitionNode::~CFunctionDefinitionNode()
{
	
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
	std::map<std::string,CVariableEntry>::iterator	foundVariable = mLocals.find( inName );
	if( foundVariable == mLocals.end() )
		mLocals[inName] = newEntry;
	if( isGlobal )
	{
		foundVariable = (*mGlobals).find( inName );
		if( foundVariable == (*mGlobals).end() )
			(*mGlobals)[inName] = newEntry;
	}
}


long	CFunctionDefinitionNode::GetBPRelativeOffsetForLocalVar( const std::string& inName )
{
	std::map<std::string,CVariableEntry>::iterator	foundVariable = mLocals.find( inName );
	if( foundVariable != mLocals.end() )
	{
		long	bpRelOffs = foundVariable->second.mBPRelativeOffset;
		if( bpRelOffs == LONG_MAX )
		{
			bpRelOffs = mLocalVariableCount++;
			bpRelOffs = -bpRelOffs;
			foundVariable->second.mBPRelativeOffset = bpRelOffs;
		}
		return bpRelOffs;
	}
	else
		return LONG_MAX;
}


void	CFunctionDefinitionNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Function " << mName << std::endl;
	
	DebugPrintInner( destStream, indentLevel );
}



void	CFunctionDefinitionNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GenerateFunctionPrologForName( mName, GetLocalVariableCount() );
	
	CCodeBlockNodeBase::GenerateCode( inCodeBlock );
	
	inCodeBlock->GenerateFunctionEpilogForName( mName, GetLocalVariableCount() );
}

} /* namespace Carlson */