/*
 *  CGlobalPropertyNode.h
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


class CGlobalPropertyNode : public CValueNode
{
public:
	CGlobalPropertyNode( CParseTree* inTree, LEOInstructionID inSetterInstructionID, LEOInstructionID inGetterInstructionID, size_t inLineNum )
		: CValueNode(inTree), mSetterInstructionID(inSetterInstructionID), mGetterInstructionID(inGetterInstructionID), mLineNum(inLineNum) {};
	virtual ~CGlobalPropertyNode() {};
	
	virtual size_t			GetParamCount()									{ return mParams.size(); };
	virtual CValueNode*		GetParamAtIndex( size_t idx )					{ return mParams[idx]; };
	virtual void			SetParamAtIndex( size_t idx, CValueNode* val )	{ mParams[idx] = val; };
	virtual void			AddParam( CValueNode* val );
	
	virtual void			DebugPrint( std::ostream& destStream, size_t indentLevel );

	virtual void			Simplify();
	virtual void			GenerateCode( CCodeBlock* inCodeBlock );
	virtual void			GenerateSetterCode( CCodeBlock* inCodeBlock, CValueNode* newValueNode );

protected:
	LEOInstructionID			mSetterInstructionID;
	LEOInstructionID			mGetterInstructionID;
	std::vector<CValueNode*>	mParams;
	size_t						mLineNum;
};

}
