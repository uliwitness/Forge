/*
 *  CParseErrorCommandNode.h
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"

namespace Carlson
{

class CParseErrorCommandNode : public CCommandNode
{
public:
	CParseErrorCommandNode( CParseTree* inTree, const std::string inErrorMessage, const std::string inFileName, size_t inLineNum, size_t inOffset )
	: CCommandNode( inTree, "parseError", inLineNum ), mFileName(inFileName), mErrorMessage(inErrorMessage), mOffset(inOffset) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );

	virtual void	DebugPrint( std::ostream& destStream, size_t indentLevel );

protected:
	std::string		mFileName;
	std::string		mErrorMessage;
	size_t			mOffset;
};

} // namespace Carlson
