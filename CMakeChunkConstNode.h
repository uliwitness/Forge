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
	CMakeChunkConstNode( CParseTree* inTree, CCodeBlockNodeBase * currFunc, size_t inLineNum )
		: CFunctionCallNode( inTree, false, "MakeChunkConst", inLineNum ), mCurrFunc(currFunc) {};
		
	virtual CValueNode*	Copy();

	virtual void		GenerateCode( CCodeBlock* inCodeBlock );
	
	virtual CCodeBlockNodeBase*	GetCurrentFunction()	{ return mCurrFunc; };

protected:
	CCodeBlockNodeBase	*	mCurrFunc;
};


}	// namespace Carlson
