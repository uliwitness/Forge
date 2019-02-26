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
#include "CNodeTransformation.h"

namespace Carlson
{

bool	CCodeBlockNodeBase::LocalVariableExists( const std::string& inStr )
{
	std::map<std::string,CVariableEntry>&	locals = GetLocals();
	return locals.find( inStr ) != locals.end();
}


void	CCodeBlockNodeBase::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Code Block" << std::endl;
	
	DebugPrintInner( destStream, indentLevel );
}


void	CCodeBlockNodeBase::DebugPrintInner( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "{" << std::endl;
	
	std::vector<CNode*>::iterator itty;
	
	for( itty = mCommands.begin(); itty != mCommands.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +1 );
	}
	
	destStream << indentChars << "}" << std::endl;
}


void	CCodeBlockNodeBase::Simplify()
{
	CNode::Simplify();
	
	std::vector<CNode*>::iterator itty;
	
	for( itty = mCommands.begin(); itty != mCommands.end(); itty++ )
	{
		CNode	*	originalNode = *itty;
		originalNode->Simplify();	// Give subnodes a chance to apply transformations first. Might expose simpler sub-nodes we can then simplify.
		CNode* newNode = CNodeTransformationBase::Apply( originalNode );	// Returns either originalNode, or a totally new object, in which case we delete the old one.
		if( newNode != originalNode )
			*itty = newNode;
	}
}


void	CCodeBlockNodeBase::Visit( std::function<void(CNode*)> visitorBlock )
{
	for( auto currCommand : mCommands )
		currCommand->Visit( visitorBlock );
	
	CNode::Visit( visitorBlock );
}


void	CCodeBlockNodeBase::GenerateCode( CCodeBlock* inCodeBlock )
{
	std::vector<CNode*>::iterator itty;
	
	for( itty = mCommands.begin(); itty != mCommands.end(); itty++ )
	{
		(*itty)->GenerateCode( inCodeBlock );
	}
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
								isParam, isGlobal || GetAllVarsAreGlobals(), dontDispose );
	std::map<std::string,CVariableEntry>::iterator	foundVariable = (*mLocals).find( inName );
	if( foundVariable == (*mLocals).end() )
		(*mLocals)[inName] = newEntry;
	if( isGlobal || GetAllVarsAreGlobals() )
	{
		foundVariable = (*mGlobals).find( inName );
		if( foundVariable == (*mGlobals).end() )
			(*mGlobals)[inName] = newEntry;
	}
}


int16_t	CCodeBlockNode::GetBPRelativeOffsetForLocalVar( const std::string& inName )
{
	std::map<std::string,CVariableEntry>::iterator	foundVariable = (*mLocals).find( inName );
	if( foundVariable != (*mLocals).end() )
	{
		int16_t	bpRelOffs = foundVariable->second.mBPRelativeOffset;
		if( bpRelOffs == INT16_MAX)
		{
			bpRelOffs = (*mLocalVariableCount)++;
			foundVariable->second.mBPRelativeOffset = bpRelOffs;
		}
		return bpRelOffs;
	}
	else
		return INT16_MAX;
}



} // namespace Carlson
