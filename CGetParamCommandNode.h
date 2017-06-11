/*
 *  CGetParamCommandNode.h
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"

namespace Carlson
{

class CGetParamCommandNode : public CCommandNode
{
public:
	CGetParamCommandNode( CParseTree* inTree, size_t inLineNum, std::string inFileName )
		: CCommandNode( inTree, "GetParameter", inLineNum, inFileName ) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
};

} // namespace Carlson
