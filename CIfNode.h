/*
 *  CIfNode.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 19.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CCodeBlockNode.h"

namespace Carlson
{

class CIfNode : public CCodeBlockNode
{
public:
	CIfNode( CParseTree* inTree, size_t inLineNum, const std::string &inFileName, CCodeBlockNodeBase* owningBlock ) : CCodeBlockNode( inTree, inLineNum, inFileName, owningBlock ), mCondition(NULL), mElseBlock(NULL), mThenLineNum(0), mIfCommandsLineNum(0), mElseLineNum(0), mElseCommandsLineNum(0), mEndIfLineNum(0) {};
	~CIfNode() { delete mCondition; mCondition = NULL; if( mElseBlock ) { delete mElseBlock; mElseBlock = NULL; } };

	virtual void			SetCondition( CValueNode* inCond )	{ if( mCondition ) delete mCondition; mCondition = inCond; };	// inCond is now owned by the CIfNode.
	virtual CCodeBlockNode*	CreateElseBlock( size_t inLineNum )	{ mElseBlock = new CCodeBlockNode( mParseTree, inLineNum, mFileName, mOwningBlock ); return mElseBlock; };
	virtual CCodeBlockNode*	GetElseBlock()						{ return mElseBlock; };	// May return NULL!
	
	virtual void			DebugPrint( std::ostream& destStream, size_t indentLevel );
	virtual void			GenerateCode( CCodeBlock* inBlock );
	virtual void			Simplify();
	virtual void			Visit( std::function<void(CNode*)> visitorBlock );
	
	size_t	GetThenLineNum()					{ return mThenLineNum; };
	void	SetThenLineNum( size_t n )			{ mThenLineNum = n; };
	size_t	GetIfCommandsLineNum()				{ return mIfCommandsLineNum; };
	void	SetIfCommandsLineNum( size_t n )	{ mIfCommandsLineNum = n; };
	size_t	GetElseLineNum()					{ return mElseLineNum; };
	void	SetElseLineNum( size_t n )			{ mElseLineNum = n; };
	size_t	GetElseCommandsLineNum()			{ return mElseCommandsLineNum; };
	void	SetElseCommandsLineNum( size_t n )	{ mElseCommandsLineNum = n; };
	size_t	GetEndIfLineNum()					{ return mEndIfLineNum; };
	void	SetEndIfLineNum( size_t n )			{ mEndIfLineNum = n; };
	
protected:
	CCodeBlockNode*	mElseBlock;
	CValueNode*		mCondition;
	size_t			mThenLineNum;
	size_t			mIfCommandsLineNum;
	size_t			mElseLineNum;
	size_t			mElseCommandsLineNum;
	size_t			mEndIfLineNum;
};

}
