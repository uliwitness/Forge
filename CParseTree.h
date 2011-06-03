/*
 *  CParseTree.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

#include "CNode.h"
#include "CVariableEntry.h"
#include <deque>
#include <map>
#include <string>


namespace Carlson
{

class CParseTree;

class CParseTree
{
public:
	virtual ~CParseTree();
	
	virtual void		AddNode( CNode* inNode )			{ mNodes.push_back( inNode ); };
	void				NodeWasAdded( CNode* inNode )		{ };
	
	std::map<std::string,CVariableEntry>&	GetGlobals()	{ return mGlobals; };
	
	virtual void		Simplify();
	virtual void		GenerateCode( CCodeBlock* inCodeBlock );
	
	virtual void		DebugPrint( std::ostream& destStream, size_t indentLevel );

protected:
	std::deque<CNode*>						mNodes;	// The tree owns any nodes you add and will delete them when it goes out of scope.
	std::map<std::string,CVariableEntry>	mGlobals;
};

}
