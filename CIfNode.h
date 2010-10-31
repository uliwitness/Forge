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
	CIfNode( CParseTree* inTree, size_t inLineNum, CCodeBlockNodeBase* owningBlock ) : CCodeBlockNode( inTree, inLineNum, owningBlock ), mElseBlock(NULL) {};
	~CIfNode() { delete mCondition; mCondition = NULL; if( mElseBlock ) { delete mElseBlock; mElseBlock = NULL; } };

	virtual void			SetCondition( CValueNode* inCond )	{ if( mCondition ) delete mCondition; mCondition = inCond; };	// inCond is now owned by the CIfNode.
	virtual CCodeBlockNode*	CreateElseBlock( size_t inLineNum )	{ mElseBlock = new CCodeBlockNode( mParseTree, inLineNum, mOwningBlock ); return mElseBlock; };
	virtual CCodeBlockNode*	GetElseBlock()						{ return mElseBlock; };	// May return NULL!
	
protected:
	CCodeBlockNode*	mElseBlock;
	CValueNode*		mCondition;
};

}