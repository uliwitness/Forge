/*
 *  CGetArrayItemCountNode.h
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*
	GetArrayItemCount( destVar, srcVar )
*/

#include "CCommandNode.h"


namespace Carlson
{

class CGetArrayItemCountNode : public CCommandNode
{
public:
	CGetArrayItemCountNode( CParseTree* inTree, size_t inLineNum, std::string inFileName )
		: CCommandNode( inTree, "GetArrayItemCount", inLineNum, inFileName ) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
};

} // namespace Carlson
