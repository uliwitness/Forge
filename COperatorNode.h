/*
 *  COperatorNode.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

#include "CValueNode.h"
#include <vector>
extern "C" {
#include "LEOInterpreter.h"
}

namespace Carlson
{

class CValueNode;

class COperatorNode : public CValueNode
{
public:
	COperatorNode( CParseTree* inTree, LEOInstructionID inInstructionID, size_t inLineNum )
		: CValueNode(inTree,inLineNum), mInstructionID(inInstructionID),
		mInstructionParam1(0), mInstructionParam2(0) {};
	virtual ~COperatorNode() {};
	
	virtual size_t		GetParamCount()									{ return mParams.size(); };
	virtual CValueNode*	GetParamAtIndex( size_t idx )					{ return mParams[idx]; };
	virtual void		SetParamAtIndex( size_t idx, CValueNode* val );	// Doesn't free previous value. Takes over ownership of new one.
	virtual void		AddParam( CValueNode* val );
	
	virtual CValueNode*	Copy();

	virtual void		DebugPrint( std::ostream& destStream, size_t indentLevel );
	virtual const char*	GetDebugNodeName()	{ return "Operator Call"; };

	virtual void		Simplify();
	virtual void		GenerateCode( CCodeBlock* inCodeBlock );
	virtual void		Visit( std::function<void(CNode*)> visitorBlock );
	
	virtual void		SetInstructionID( LEOInstructionID inID )					{ mInstructionID = inID; };
	virtual LEOInstructionID	GetInstructionID()									{ return mInstructionID; };
	virtual void		SetInstructionParams( uint16_t inParam1, uint32_t inParam2 ){ mInstructionParam1 = inParam1; mInstructionParam2 = inParam2; };

protected:
	LEOInstructionID			mInstructionID;
	uint16_t					mInstructionParam1;
	uint32_t					mInstructionParam2;
	std::vector<CValueNode*>	mParams;
};

}
