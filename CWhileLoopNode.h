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
	CWhileLoopNode( CParseTree* inTree, size_t inLineNum, CCodeBlockNodeBase* owningBlock ) : CCodeBlockNode( inTree, inLineNum, owningBlock ), mCondition(NULL), mCommandsLineNum(0), mEndRepeatLineNum(0) {};
	~CWhileLoopNode() { if( mCondition) delete mCondition; mCondition = NULL; };

	virtual void	SetCondition( CValueNode* inCond )	{ if( mCondition ) delete mCondition; mCondition = inCond; };	// inCond is now owned by the CWhileLoopNode.
	
	virtual void	GenerateCode( CCodeBlock* inBlock );
	virtual void	Simplify();
	virtual void	Visit( std::function<void(CNode*)> visitorBlock );
	
	virtual void	DebugPrint( std::ostream& destStream, size_t indentLevel );
	
	size_t	GetCommandsLineNum()			{ return mCommandsLineNum; };
	void	SetCommandsLineNum( size_t n )	{ mCommandsLineNum = n; };
	size_t	GetEndRepeatLineNum()			{ return mEndRepeatLineNum; };
	void	SetEndRepeatLineNum( size_t n )	{ mEndRepeatLineNum = n; };
	
protected:
	CValueNode*		mCondition;
	size_t			mCommandsLineNum;
	size_t			mEndRepeatLineNum;
};

}
