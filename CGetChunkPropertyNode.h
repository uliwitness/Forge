//
//  CGetChunkPropertyNode.h
//  Forge
//
//  Created by Uli Kusterer on 03.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

#include "CFunctionCallNode.h"


namespace Carlson
{

class CGetChunkPropertyNode : public CFunctionCallNode
{
public:
		CGetChunkPropertyNode( CParseTree* inTree, std::string inPropName, size_t inLineNum )
			: CFunctionCallNode( inTree, false, inPropName, inLineNum ) {};
		
	virtual CValueNode*	Copy();

	virtual void		GenerateCode( CCodeBlock* inCodeBlock );
	
protected:
	virtual const char*	GetNodeName()		{ return "GetChunkProperty"; };
};


}	// namespace Carlson
