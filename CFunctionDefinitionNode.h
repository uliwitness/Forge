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
	CFunctionDefinitionNode( CParseTree* inTree, bool isCommand, const std::string& inName, const std::string& userHandlerName, size_t inLineNum )
		: CCodeBlockNodeBase( inTree, inLineNum ), mName( inName ), mUserHandlerName(userHandlerName), mLineNum( inLineNum ), mEndLineNum(0), mLocalVariableCount(0), mIsCommand(isCommand), mAllVarsAreGlobals(false)
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
	virtual std::map<std::string,CVariableEntry>&		GetGlobals()			{ return mGlobals; };
	
	virtual void	DebugPrint( std::ostream& destStream, size_t indentLevel );
	
	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
	
	void			SetEndLineNum( size_t inEndLineNum )	{ mEndLineNum = inEndLineNum; };	// Line number of function's "end" marker, so we can indicate end to the debugger.
	size_t			GetEndLineNum()							{ return mEndLineNum; };
	void			SetCommandsLineNum( size_t n )			{ mCommandsLineNum = n; };
	size_t			GetCommandsLineNum()					{ return mCommandsLineNum; };
	
	void			SetAllVarsAreGlobals( bool inState )	{ mAllVarsAreGlobals = inState; }
	
	virtual CCodeBlockNodeBase*	GetContainingFunction()		{ return this; };
	bool						IsCommand()					{ return mIsCommand; };
	const std::string&			GetUserHandlerName()		{ return mUserHandlerName; };
	
protected:
	std::string								mName;
	std::string								mUserHandlerName;
	bool									mIsCommand;
	size_t									mLineNum;
	size_t									mEndLineNum;
	size_t									mCommandsLineNum;
	std::map<std::string,CVariableEntry>	mLocals;
	size_t									mLocalVariableCount;
	std::map<std::string,CVariableEntry>	mGlobals;
	bool									mAllVarsAreGlobals;	// For message box, which doesn't have local variables and creates globals for every local var.
};


}
