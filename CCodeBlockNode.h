/*
 *  CCodeBlockNode.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

#include "CValueNode.h"
#include "CParser.h"
#include <vector>


namespace Carlson
{

class CCommandNode;


// Used for top-level code blocks and the type of params to objects that accept
// a code block:
//	This is an abstract class. You need to implement AddLocalVar() and GetLocals()
//	yourself, to do whatever is appropriate for your kind of block:
class CCodeBlockNodeBase : public CNode
{
public:
	CCodeBlockNodeBase( size_t inLineNum, std::map<std::string,CVariableEntry>* inGlobals )
		: CNode(), mLineNum( inLineNum ), mGlobals(inGlobals) {};
	virtual ~CCodeBlockNodeBase();
	
	virtual void	AddCommand( CNode* inCmd )	{ mCommands.push_back( inCmd ); };	// Function node now owns this command and will delete it!
	
	virtual void	AddLocalVar( const std::string& inName, const std::string& inUserName,
									TVariantType theType, bool initWithName = false,
									bool isParam = false, bool isGlobal = false,
									bool dontDispose = false ) = 0;

	virtual std::map<std::string,CVariableEntry>&		GetLocals() = 0;
	virtual std::map<std::string,CVariableEntry>&		GetGlobals()	{ return *mGlobals; };
		
	virtual void	DebugPrint( std::ostream& destStream, size_t indentLevel );
	
protected:
	virtual void	DebugPrintInner( std::ostream& destStream, size_t indentLevel );
	
protected:
	size_t									mLineNum;
	std::vector<CNode*>						mCommands;
	std::map<std::string,CVariableEntry>*	mGlobals;
};


// This is a concrete code block. It simply groups together a couple of commands
//	and changes the local and global variable lists of the "owning" block you pass in:
class CCodeBlockNode : public CCodeBlockNodeBase
{
public:
	CCodeBlockNode( size_t inLineNum, CCodeBlockNodeBase* owningBlock )
		: CCodeBlockNodeBase( inLineNum, NULL ), mOwningBlock(NULL)
	{
		mOwningBlock = owningBlock;
		mGlobals = &owningBlock->GetGlobals();
		mLocals = &owningBlock->GetLocals();
	};
	virtual ~CCodeBlockNode()	{};

	virtual void	AddLocalVar( const std::string& inName, const std::string& inUserName,
									TVariantType theType, bool initWithName = false,
									bool isParam = false, bool isGlobal = false,
									bool dontDispose = false );

	virtual std::map<std::string,CVariableEntry>&		GetLocals()	{ return *mLocals; };
	
protected:
	std::map<std::string,CVariableEntry>*	mLocals;
	CCodeBlockNodeBase*						mOwningBlock;
};

}