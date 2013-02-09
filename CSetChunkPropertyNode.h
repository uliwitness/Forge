//
//  CSetChunkPropertyNode.h
//  Forge
//
//  Created by Uli Kusterer on 03.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "CFunctionCallNode.h"


namespace Carlson
{

class CSetChunkPropertyNode : public CFunctionCallNode
{
public:
		CSetChunkPropertyNode( CParseTree* inTree, size_t inLineNum )
			: CFunctionCallNode( inTree, false, "SetChunkProperty", inLineNum ) {};
		
	virtual CValueNode*	Copy();

	virtual void		GenerateCode( CCodeBlock* inCodeBlock );
};


}	// namespace Carlson
