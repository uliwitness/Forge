//
//  CMakeChunkConstNode.h
//  Forge
//
//  Created by Uli Kusterer on 03.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "CFunctionCallNode.h"


namespace Carlson
{

class CMakeChunkConstNode : public CFunctionCallNode
{
public:
		CMakeChunkConstNode( CParseTree* inTree, size_t inLineNum )
			: CFunctionCallNode( inTree, false, "MakeChunkConst", inLineNum ) {};
		
	virtual CValueNode*	Copy();

	virtual void		GenerateCode( CCodeBlock* inCodeBlock );
};


}	// namespace Carlson
