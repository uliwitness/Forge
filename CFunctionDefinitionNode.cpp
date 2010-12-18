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


size_t	CFunctionDefinitionNode::GetBPRelativeOffsetForLocalVar( const std::string& inName )
{
	std::map<std::string,CVariableEntry>::iterator	foundVariable = mLocals.find( inName );
	if( foundVariable != mLocals.end() )
	{
		size_t	bpRelOffs = foundVariable->second.mBPRelativeOffset;
		if( bpRelOffs == SIZE_MAX )
		{
			foundVariable->second.mBPRelativeOffset = mLocalVariableCount++;
			bpRelOffs = foundVariable->second.mBPRelativeOffset;
		}
		return bpRelOffs;
	}
	else
		return SIZE_MAX;
}


void	CFunctionDefinitionNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Function " << mName << std::endl;
	
	DebugPrintInner( destStream, indentLevel );
}



void	CFunctionDefinitionNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GenerateFunctionPrologForName( mName );
	
	CCodeBlockNodeBase::GenerateCode( inCodeBlock );
	
	inCodeBlock->GenerateFunctionEpilogForName( mName );
}

} /* namespace Carlson */