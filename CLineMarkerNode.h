/*
 *  CLineMarkerNode.h
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"

namespace Carlson
{

class CLineMarkerNode : public CCommandNode
{
public:
	CLineMarkerNode( CParseTree* inTree, size_t inLineNum )
		: CCommandNode( inTree, "LINE:", inLineNum ) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
};

} // namespace Carlson