//
//  CChunkPropertyNodeTransformation.h
//  Stacksmith
//
//  Created by Uli Kusterer on 28.05.12.
//  Copyright (c) 2012 Uli Kusterer. All rights reserved.
//

#include "CNodeTransformation.h"
#include "CObjectPropertyNode.h"
#include "CPutCommandNode.h"


namespace Carlson
{


class CChunkPropertyNodeTransformation : public CNodeTransformation<CObjectPropertyNode>
{
public:
	virtual CNode*	Simplify( CObjectPropertyNode* inPropNode );

	static void		Initialize()	{ sNodeTransformations.push_back( new CChunkPropertyNodeTransformation ); };
};

class CChunkPropertyPutNodeTransformation : public CNodeTransformation<CPutCommandNode>
{
public:
	virtual CNode*	Simplify( CPutCommandNode* inPropNode );

	static void		Initialize()	{ sNodeTransformations.push_back( new CChunkPropertyPutNodeTransformation ); };
};

} // namespace Carlson