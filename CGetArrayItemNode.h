/*
 *  CGetArrayItemNode.h
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*
	GetArrayItemCount( destVar, key, srcVar )
*/

#include "CCommandNode.h"


namespace Carlson
{

class CGetArrayItemNode : public CCommandNode
{
public:
	CGetArrayItemNode( CParseTree* inTree, size_t inLineNum )
		: CCommandNode( inTree, "GetArrayItem", inLineNum ) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
};

} // namespace Carlson
