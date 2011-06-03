/*
 *  CAssignChunkArrayNode.h
 *  Forge
 *
 *  Created by Uli Kusterer on 18.12.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*
	AssignChunkArray( destVar, chunkType (int), srcVar )
*/

#include "CCommandNode.h"


namespace Carlson
{

class CAssignChunkArrayNode : public CCommandNode
{
public:
	CAssignChunkArrayNode( CParseTree* inTree, size_t inLineNum )
		: CCommandNode( inTree, "AssignChunkArray", inLineNum ) {};

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );
};

} // namespace Carlson
