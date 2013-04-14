//
//  CConcatOperatorNodeTransformation.h
//  Stacksmith
//
//  Created by Uli Kusterer on 16.03.2013.
//  Copyright (c) 2013 Uli Kusterer. All rights reserved.
//

#include "COperatorNodeTransformation.h"
#include "COperatorNode.h"


namespace Carlson
{


class CConcatOperatorNodeTransformation : public COperatorNodeTransformation
{
public:
	virtual CNode*	Simplify( COperatorNode* inPropNode );
	
	virtual LEOInstructionID	GetInstructionID();
	
	static void		Initialize()	{ sNodeTransformations.push_back( new CConcatOperatorNodeTransformation ); };
};


} // namespace Carlson