/*
 *  CObjectPropertyNode.h
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


class CObjectPropertyNode : public CValueNode
{
public:
	CObjectPropertyNode( CParseTree* inTree, const std::string& inSymbolName, size_t inLineNum )
		: CValueNode(inTree,inLineNum), mSymbolName(inSymbolName) {};
	virtual ~CObjectPropertyNode() {};
	
	virtual void		GetSymbolName( std::string& outSymbolName )		{ outSymbolName = mSymbolName; };
	virtual size_t		GetParamCount()									{ return mParams.size(); };
	virtual CValueNode*	GetParamAtIndex( size_t idx )					{ return mParams[idx]; };
	virtual void		SetParamAtIndex( size_t idx, CValueNode* val )	{ mParams[idx] = val; };
	virtual void		AddParam( CValueNode* val );

	virtual CValueNode*	Copy();
	
	virtual void		DebugPrint( std::ostream& destStream, size_t indentLevel );

	virtual void		Simplify();
	virtual void		GenerateCode( CCodeBlock* inCodeBlock );
	virtual void		Visit( std::function<void(CNode*)> visitorBlock );

protected:
	std::string					mSymbolName;
	std::vector<CValueNode*>	mParams;
};

}
