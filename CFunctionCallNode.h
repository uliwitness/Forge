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


namespace Carlson
{

class CValueNode;


class CFunctionCallNode : public CValueNode
{
public:
	CFunctionCallNode( const std::string& inSymbolName, size_t inLineNum ) : CValueNode(), mSymbolName(inSymbolName), mLineNum(inLineNum) {};
	virtual ~CFunctionCallNode() {};
	
	virtual void		GetSymbolName( std::string& outSymbolName )		{ outSymbolName = mSymbolName; };
	virtual size_t		GetParamCount()									{ return mParams.size(); };
	virtual CValueNode*	GetParamAtIndex( size_t idx )					{ return mParams[idx]; };
	virtual void		SetParamAtIndex( size_t idx, CValueNode* val )	{ mParams[idx] = val; };
	virtual void		AddParam( CValueNode* val )						{ mParams.push_back( val ); };
	
	virtual void		FillOutParamEntry( ParamEntry& par )			{ par = kPointerParam; };
	virtual void		GenerateCodeWithReturnOffset( CodeBlock& codeBlock, ESPRelativeOffset resultOffset );
	virtual void		GenerateCode( CodeBlock& codeBlock );
	virtual void		GenerateCpp( CppBlock& codeBlock );
	virtual void		DebugPrint( std::ostream& destStream, size_t indentLevel );

protected:
	std::string					mSymbolName;
	std::vector<CValueNode*>	mParams;
	size_t						mLineNum;
};

}