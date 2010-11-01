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
	CFunctionDefinitionNode( CParseTree* inTree, const std::string& inName, size_t inLineNum, std::map<std::string,CVariableEntry>& inGlobals )
		: CCodeBlockNodeBase( inTree, inLineNum, NULL ), mName( inName ), mLineNum( inLineNum )
	{
		
	};
	virtual ~CFunctionDefinitionNode();
		
	virtual void	AddLocalVar( const std::string& inName, const std::string& inUserName,
									TVariantType theType, bool initWithName = false,
									bool isParam = false, bool isGlobal = false,
									bool dontDispose = false );
	
	virtual std::map<std::string,CVariableEntry>&		GetLocals()		{ return mLocals; };

	virtual void	DebugPrint( std::ostream& destStream, size_t indentLevel );
	
protected:
	std::string								mName;
	size_t									mLineNum;
	std::map<std::string,CVariableEntry>	mLocals;
};


}