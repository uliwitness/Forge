/*
 *  CAddCommandNode.h
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"

namespace Carlson
{

class CAddCommandNode : public CCommandNode
{
public:
	CAddCommandNode( CParseTree* inTree, size_t inLineNum, std::string inFileName ) : CCommandNode( inTree, "AddTo", inLineNum, inFileName ) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
};

} // namespace Carlson
