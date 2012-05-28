//
//  CNodeTransformation.h
//  Stacksmith
//
//  Created by Uli Kusterer on 26.05.12.
//  Copyright (c) 2012 Uli Kusterer. All rights reserved.
//

#pragma once

/*
	CNodeTransformation is a class that you can use to implement transformations
	on a particular class of node. E.g. optimizations, or to detect certain
	patterns and re-arrange nodes, like we do for properties on chunk expressions,
	which get parsed naively, and then later transformed into the proper
	nodes.
	
	We could do this in CNode's Simplify() directly, but this way each node only
	needs to know about itself, and ONLY transformations need to know about the
	other nodes they operate on. Moreover, each transformation can be its own
	file and only THAT has its dependencies.
*/

#include "CNode.h"
#include <vector>


namespace Carlson
{


// Base class for transformations: (Generally you want CNodeTransformation below!)

class CNodeTransformationBase
{
public:
	virtual ~CNodeTransformationBase() {};

	virtual void	Simplify_External( CNode *inNode ) = 0;
	
	static void		Apply( CNode* inNode );
};


extern std::vector<CNodeTransformationBase*>		sNodeTransformations;


/*
	CNodeTransformation is templated based on the class of the node to operate
	on. Call Initialize some time at startup to register, override Simplify().
*/

template <class c_node_subclass>
class CNodeTransformation : public CNodeTransformationBase
{
public:
	virtual ~CNodeTransformation() {};
	
	virtual void	Simplify( c_node_subclass * inNode ) = 0;
	
	virtual void	Simplify_External( CNode *inNode )
	{
		c_node_subclass * subclassPtr = dynamic_cast<c_node_subclass*>(inNode);
		
		if( subclassPtr )
		{
			Simplify( subclassPtr );
		}
	}
	
	//static void		Initialize()	{ sNodeTransformations.push_back( new MYCLASSNAME ); };
};


} // Carlson
