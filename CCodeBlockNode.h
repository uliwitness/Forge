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
	CCodeBlockNodeBase( CParseTree* inTree, size_t inLineNum )
		: CNode(inTree), mLineNum( inLineNum ) {};
	virtual ~CCodeBlockNodeBase();
	
	virtual void	AddCommand( CNode* inCmd )	{ mCommands.push_back( inCmd ); mParseTree->NodeWasAdded( inCmd ); };	// Function node now owns this command and will delete it!
	
	virtual void	AddLocalVar( const std::string& inName, const std::string& inUserName,
									TVariantType theType, bool initWithName = false,
									bool isParam = false, bool isGlobal = false,
									bool dontDispose = false ) = 0;	// It's OK to call this several times with the same variable. Subsequent calls will be ignored.
	virtual long	GetBPRelativeOffsetForLocalVar( const std::string& inName ) = 0;
	
	// Sub-blocks retrieve and modify these three as needed: // TODO: This isn't really very OO.
	virtual size_t&										GetLocalVariableCount() = 0;
	virtual std::map<std::string,CVariableEntry>&		GetLocals() = 0;
	virtual std::map<std::string,CVariableEntry>&		GetGlobals() = 0;
		
	virtual void	DebugPrint( std::ostream& destStream, size_t indentLevel );

	virtual void	Simplify();
	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
	
	virtual void	DebugPrintInner( std::ostream& destStream, size_t indentLevel );
	
protected:
	size_t									mLineNum;
	std::vector<CNode*>						mCommands;
};


// This is a concrete code block. It simply groups together a couple of commands
//	and changes the local and global variable lists of the "owning" block you pass in:
class CCodeBlockNode : public CCodeBlockNodeBase
{
public:
	CCodeBlockNode( CParseTree* inTree, size_t inLineNum, CCodeBlockNodeBase* owningBlock )
		: CCodeBlockNodeBase( inTree, inLineNum ), mOwningBlock(owningBlock)
	{
		mGlobals = &owningBlock->GetGlobals();
		mLocals = &owningBlock->GetLocals();
		mLocalVariableCount = &owningBlock->GetLocalVariableCount();
	};
	virtual ~CCodeBlockNode()	{};

	virtual void	AddLocalVar( const std::string& inName, const std::string& inUserName,
									TVariantType theType, bool initWithName = false,
									bool isParam = false, bool isGlobal = false,
									bool dontDispose = false );
	virtual long	GetBPRelativeOffsetForLocalVar( const std::string& inName );

	// Sub-blocks retrieve and modify these two as needed: // TODO: This isn't really very OO.
	virtual size_t&										GetLocalVariableCount()	{ return *mLocalVariableCount; };
	virtual std::map<std::string,CVariableEntry>&		GetLocals()				{ return *mLocals; };
	virtual std::map<std::string,CVariableEntry>&		GetGlobals()			{ return *mGlobals; };
	
protected:
	std::map<std::string,CVariableEntry>*	mLocals;
	size_t*									mLocalVariableCount;
	CCodeBlockNodeBase*						mOwningBlock;
	std::map<std::string,CVariableEntry>*	mGlobals;
};

}