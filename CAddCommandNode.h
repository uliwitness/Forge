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
	CAddCommandNode( CParseTree* inTree, size_t inLineNum ) : CCommandNode( inTree, "AddTo", inLineNum ) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
};

} // namespace Carlson
