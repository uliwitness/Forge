//
//  COperatorNodeTransformation.h
//  Stacksmith
//
//  Created by Uli Kusterer on 16.03.2013.
//  Copyright (c) 2013 Uli Kusterer. All rights reserved.
//

#include "CNodeTransformation.h"
#include "COperatorNode.h"


namespace Carlson
{


class COperatorNodeTransformation : public CNodeTransformation<COperatorNode>
{
public:
	//virtual CNode*	Simplify( COperatorNode* inPropNode );
	
	virtual LEOInstructionID	GetInstructionID()	{ return 0; };
	
	virtual CNode*				Simplify_External( CNode* inNode );	// If it doesn't return 'this', caller will delete 'this' and use the optimized value.

	static void		Initialize()	{ /*sNodeTransformations.push_back( new CLASS_NAME );*/ };
};


} // namespace Carlson