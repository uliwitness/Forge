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
	//printf("poof.\n");
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
								isParam, isGlobal || GetAllVarsAreGlobals(), dontDispose );
	std::map<std::string,CVariableEntry>::iterator	foundVariable = mLocals.find( inName );
	if( foundVariable == mLocals.end() )
		mLocals[inName] = newEntry;
	if( isGlobal || GetAllVarsAreGlobals() )
	{
		foundVariable = mGlobals.find( inName );
		if( foundVariable == mGlobals.end() )
			mGlobals[inName] = newEntry;
	}
}


int16_t	CFunctionDefinitionNode::GetBPRelativeOffsetForLocalVar( const std::string& inName )
{
	std::map<std::string,CVariableEntry>::iterator	foundVariable = mLocals.find( inName );
	if( foundVariable != mLocals.end() )
	{
		int16_t	bpRelOffs = foundVariable->second.mBPRelativeOffset;
		if( bpRelOffs == INT16_MAX )
		{
			bpRelOffs = mLocalVariableCount++;
			foundVariable->second.mBPRelativeOffset = bpRelOffs;
		}
		return bpRelOffs;
	}
	else
		return INT16_MAX;
}


void	CFunctionDefinitionNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << (mIsCommand ? "Command " : "Function ") << mName << std::endl;
	
	DebugPrintInner( destStream, indentLevel );
}



void	CFunctionDefinitionNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GenerateFunctionPrologForName( mIsCommand, mName, mLocals, mLineNum );
	
	CCodeBlockNodeBase::GenerateCode( inCodeBlock );
	
	inCodeBlock->GenerateFunctionEpilogForName( mIsCommand, mName, mLocals, mEndLineNum );
}

} /* namespace Carlson */
