/*
 *  CWhileLoopNode.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 19.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CCodeBlockNode.h"


namespace Carlson
{

class CWhileLoopNode : public CCodeBlockNode
{
public:
	CWhileLoopNode( size_t inLineNum, CCodeBlockNodeBase* owningBlock ) : CCodeBlockNode( inLineNum, owningBlock ) {};
	~CWhileLoopNode() { delete mCondition; mCondition = NULL; };

	virtual void	SetCondition( CValueNode* inCond )	{ if( mCondition ) delete mCondition; mCondition = inCond; };	// inCond is now owned by the CWhileLoopNode.
	
	virtual void	GenerateCpp( CppBlock& cppBlock );
	
protected:
	CValueNode*		mCondition;
};

}