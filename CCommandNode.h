/*
 *  CCommandNode.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "CNode.h"
#include <string>
#include <vector>


// -----------------------------------------------------------------------------
//	Classes:
// -----------------------------------------------------------------------------

namespace Carlson
{

class CValueNode;


class CCommandNode : public CNode
{
public:
	CCommandNode( CParseTree* inTree, const std::string& inSymbolName, size_t inLineNum, std::string inFileName ) : CNode(inTree), mSymbolName(inSymbolName), mLineNum(inLineNum), mFileName(inFileName) {};
	virtual ~CCommandNode() {};
	
	virtual void		GetSymbolName( std::string& outSymbolName )			{ outSymbolName = mSymbolName; };
	virtual void		SetSymbolName( const std::string& inSymbolName )	{ mSymbolName = inSymbolName; };
	
	virtual size_t		GetParamCount()										{ return mParams.size(); };
	virtual CValueNode*	GetParamAtIndex( size_t idx )						{ return mParams[idx]; };
	virtual void		SetParamAtIndex( size_t idx, CValueNode* val )		{ mParams[idx] = val; };
	virtual void		AddParam( CValueNode* val );
	
	virtual void		DebugPrint( std::ostream& destStream, size_t indentLevel );
	
	virtual void		Simplify();
	virtual void		Visit( std::function<void(CNode*)> visitorBlock );
	virtual void		GenerateCode( CCodeBlock* inCodeBlock );
	
	virtual size_t		GetLineNum() const	{ return mLineNum; };
	
protected:
	std::string					mSymbolName;
	std::vector<CValueNode*>	mParams;
	size_t						mLineNum;
	std::string					mFileName;
};

} // namespace Carlson
