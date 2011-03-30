/*
 *  CFunctionCallNode.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

#include "CValueNode.h"
#include <vector>


namespace Carlson
{

class CValueNode;


class CFunctionCallNode : public CValueNode
{
public:
	CFunctionCallNode( CParseTree* inTree, bool isCommand, const std::string& inSymbolName, size_t inLineNum )
		: CValueNode(inTree), mSymbolName(inSymbolName), mLineNum(inLineNum), mIsCommand(isCommand) {};
	virtual ~CFunctionCallNode() {};
	
	virtual void		GetSymbolName( std::string& outSymbolName )		{ outSymbolName = mSymbolName; };
	virtual size_t		GetParamCount()									{ return mParams.size(); };
	virtual CValueNode*	GetParamAtIndex( size_t idx )					{ return mParams[idx]; };
	virtual void		SetParamAtIndex( size_t idx, CValueNode* val )	{ mParams[idx] = val; };
	virtual void		AddParam( CValueNode* val );

	virtual CValueNode*	Copy();
	
	virtual void		DebugPrint( std::ostream& destStream, size_t indentLevel );

	virtual void		Simplify();
	virtual void		GenerateCode( CCodeBlock* inCodeBlock );

protected:
	std::string					mSymbolName;
	bool						mIsCommand;
	std::vector<CValueNode*>	mParams;
	size_t						mLineNum;
};

}