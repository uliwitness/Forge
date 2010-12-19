/*
 *  CPrintCommandNode.h
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"

namespace Carlson
{

class CPrintCommandNode : public CCommandNode
{
public:
	CPrintCommandNode( CParseTree* inTree, size_t inLineNum )
		: CCommandNode( inTree, "PrintValue", inLineNum ) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
};

} // namespace Carlson