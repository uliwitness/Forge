/*
 *  CReturnCommandNode.h
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"

namespace Carlson
{

class CReturnCommandNode : public CCommandNode
{
public:
	CReturnCommandNode( CParseTree* inTree, size_t inLineNum, std::string inFileName )
		: CCommandNode( inTree, "return", inLineNum, inFileName ) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
};

} // namespace Carlson
