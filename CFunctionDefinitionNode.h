/*
 *  CFunctionDefinitionNode.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

#include "CCodeBlockNode.h"


namespace Carlson
{

class CCommandNode;

class CFunctionDefinitionNode : public CCodeBlockNodeBase
{
public:
	CFunctionDefinitionNode( CParseTree* inTree, bool isCommand, const std::string& inName, size_t inLineNum, std::map<std::string,CVariableEntry>& inGlobals )
		: CCodeBlockNodeBase( inTree, inLineNum, NULL ), mName( inName ), mLineNum( inLineNum ), mEndLineNum(0), mLocalVariableCount(0), mIsCommand(isCommand)
	{
		
	};
	virtual ~CFunctionDefinitionNode();
		
	virtual void	AddLocalVar( const std::string& inName, const std::string& inUserName,
									TVariantType theType, bool initWithName = false,
									bool isParam = false, bool isGlobal = false,
									bool dontDispose = false );
	virtual long	GetBPRelativeOffsetForLocalVar( const std::string& inName );
	
	// Sub-blocks retrieve and modify these two as needed: // TODO: This isn't really very OO.
	virtual size_t&										GetLocalVariableCount()	{ return mLocalVariableCount; };
	virtual std::map<std::string,CVariableEntry>&		GetLocals()				{ return mLocals; };
	
	virtual void	DebugPrint( std::ostream& destStream, size_t indentLevel );
	
	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
	
	void			SetEndLineNum( size_t inEndLineNum )	{ mEndLineNum = inEndLineNum; };	// Line number of function's "end" marker, so we can indicate end to the debugger.

	
protected:
	std::string								mName;
	bool									mIsCommand;
	size_t									mLineNum;
	size_t									mEndLineNum;
	std::map<std::string,CVariableEntry>	mLocals;
	size_t									mLocalVariableCount;
};


}